#include "SystemView.h"

#include <rhi/qrhi.h>
#include <QAbstractEventDispatcher>
#include <QImage>
#include <QImageWriter>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QPointer>
#include <QQuickItem>
#include <QSize>
#include <QString>
#include <QThread>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "RhiOffscreenContext.h"
#include "RhiToneMapper.h"
#include "core/HistogramBuffer.h"
#include "ui/storage/conversions.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit::ui {

Q_LOGGING_CATEGORY(systemViewLog, "SystemView");

class SystemView::HistogramRenderer : public QQuickRhiItemRenderer {
 public:
  explicit HistogramRenderer(SystemView *view)
      : driver_(view->driver()), viewPtr_(view), viewThread_(view->thread()) {}

  ~HistogramRenderer() override {
    failPendingExports(
        QStringLiteral("Renderer destroyed before export completed"));
    toneMapper_.releaseResources();
    exportToneMapper_.releaseResources();
  }

 protected:
  void initialize(QRhiCommandBuffer * /*cb*/) override {
    if (rhi_ != rhi()) {
      toneMapper_.releaseResources();
      exportToneMapper_.releaseResources();
      rhi_ = rhi();
      initialized_ = false;
      histogramDirty_ = true;
    }

    if (rhi_ == nullptr || renderTarget() == nullptr) {
      return;
    }

    QString error;
    if (!toneMapper_.initialize(rhi_, renderTarget()->renderPassDescriptor(),
                                renderTarget()->sampleCount(), &error)) {
      qCWarning(systemViewLog, "Failed to initialize RhiToneMapper: %s",
                qPrintable(error));
      initialized_ = false;
      return;
    }
    initialized_ = true;
  }

  void synchronize(QQuickRhiItem *item) override {
    auto *view = qobject_cast<SystemView *>(item);
    if (view == nullptr) {
      return;
    }

    if (const auto *model = view->model()) {
      gamma_ = model->gamma();
      exposure_ = model->exposure();
      vibrancy_ = model->vibrancy();
      toneMapper_.setGamma(gamma_);
      toneMapper_.setExposure(exposure_);
      toneMapper_.setVibrancy(vibrancy_);
    }

    if (driver_->syncSnapshotIfChanged(lastSnapshotVersion_, histogramCopy_,
                                       &plottedIterations_)) {
      histogramDirty_ = true;
    }

    if (!view->pendingExports_.empty()) {
      for (ExportRequest &request : view->pendingExports_) {
        pendingExports_.push_back(std::move(request));
      }
      view->pendingExports_.clear();
    }
  }

  void render(QRhiCommandBuffer *cb) override {
    if (!initialized_ || renderTarget() == nullptr) {
      failPendingExports(QStringLiteral("GPU renderer is not initialized"));
      return;
    }

    toneMapper_.render(cb, renderTarget(),
                       histogramDirty_ ? &histogramCopy_ : nullptr,
                       plottedIterations_);
    histogramDirty_ = false;

    if (!pendingExports_.empty()) {
      std::vector<ExportRequest> exports = std::move(pendingExports_);
      pendingExports_.clear();
      for (const ExportRequest &request : exports) {
        executeExport(cb, request);
      }
    }
  }

 private:
  template <typename Callback>
  void invokeOnViewThread(Callback &&callback) const {
    if (QAbstractEventDispatcher *dispatcher =
            QAbstractEventDispatcher::instance(viewThread_)) {
      QPointer<SystemView> target = viewPtr_;
      QMetaObject::invokeMethod(
          dispatcher,
          [target, fn = std::forward<Callback>(callback)]() mutable {
            if (target) {
              fn(target.data());
            }
          },
          Qt::QueuedConnection);
    }
  }

  void emitExportSucceeded(const QString &filePath) const {
    invokeOnViewThread([filePath](SystemView *view) {
      emit view->imageExportSucceeded(filePath);
    });
  }

  void emitExportFailed(const QString &filePath, const QString &error) const {
    invokeOnViewThread([filePath, error](SystemView *view) {
      emit view->imageExportFailed(filePath, error);
    });
  }

  void failPendingExports(const QString &error) {
    if (pendingExports_.empty()) {
      return;
    }
    std::vector<ExportRequest> exports = std::move(pendingExports_);
    pendingExports_.clear();
    for (const ExportRequest &request : exports) {
      emitExportFailed(request.filePath, error);
    }
  }

  void executeExport(QRhiCommandBuffer *cb, const ExportRequest &request) {
    if (rhi_ == nullptr || cb == nullptr) {
      emitExportFailed(request.filePath,
                       QStringLiteral("GPU context is unavailable"));
      return;
    }

    if (request.width == 0 || request.height == 0) {
      emitExportFailed(request.filePath,
                       QStringLiteral("Project dimensions must be positive"));
      return;
    }

    const int maxTexSize = rhi_->resourceLimit(QRhi::TextureSizeMax);
    if (maxTexSize > 0 &&
        (request.width > static_cast<uint32_t>(maxTexSize) ||
         request.height > static_cast<uint32_t>(maxTexSize))) {
      emitExportFailed(
          request.filePath,
          QStringLiteral("Project dimensions exceed GPU texture limit"));
      return;
    }

    const QSize exportSize(static_cast<int>(request.width),
                           static_cast<int>(request.height));
    std::unique_ptr<QRhiTexture> exportTexture(rhi_->newTexture(
        QRhiTexture::RGBA8, exportSize, 1,
        QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!exportTexture || !exportTexture->create()) {
      emitExportFailed(request.filePath,
                       QStringLiteral("Failed to create export GPU texture"));
      return;
    }

    std::unique_ptr<QRhiTextureRenderTarget> exportTarget(
        rhi_->newTextureRenderTarget(
            QRhiTextureRenderTargetDescription(exportTexture.get())));
    if (!exportTarget) {
      emitExportFailed(
          request.filePath,
          QStringLiteral("Failed to allocate export render target"));
      return;
    }

    std::unique_ptr<QRhiRenderPassDescriptor> exportPassDesc(
        exportTarget->newCompatibleRenderPassDescriptor());
    if (!exportPassDesc) {
      emitExportFailed(
          request.filePath,
          QStringLiteral("Failed to create export render pass descriptor"));
      return;
    }
    exportTarget->setRenderPassDescriptor(exportPassDesc.get());
    if (!exportTarget->create()) {
      emitExportFailed(request.filePath,
                       QStringLiteral("Failed to create export render target"));
      return;
    }

    QString rhiError;
    if (!exportToneMapper_.initialize(rhi_, exportPassDesc.get(), 1,
                                      &rhiError)) {
      exportToneMapper_.releaseResources();
      emitExportFailed(request.filePath,
                       rhiError.isEmpty()
                           ? QStringLiteral("Failed to initialize tone mapper")
                           : rhiError);
      return;
    }
    exportToneMapper_.setGamma(gamma_);
    exportToneMapper_.setExposure(exposure_);
    exportToneMapper_.setVibrancy(vibrancy_);

    QRhiReadbackResult readbackResult;
    bool readbackCompleted = false;
    readbackResult.completed = [&readbackCompleted]() {
      readbackCompleted = true;
    };
    QRhiResourceUpdateBatch *readbackBatch = rhi_->nextResourceUpdateBatch();
    readbackBatch->readBackTexture(QRhiReadbackDescription(exportTexture.get()),
                                   &readbackResult);

    const HistogramBuffer *histogramToUpload =
        (histogramCopy_.width() > 0 && histogramCopy_.height() > 0)
            ? &histogramCopy_
            : nullptr;
    exportToneMapper_.render(cb, exportTarget.get(), histogramToUpload,
                             plottedIterations_, readbackBatch);

    const QRhi::FrameOpResult finishResult = rhi_->finish();
    exportToneMapper_.releaseResources();
    exportTarget.reset();
    exportPassDesc.reset();
    exportTexture.reset();

    if (finishResult != QRhi::FrameOpSuccess || !readbackCompleted) {
      emitExportFailed(request.filePath,
                       QStringLiteral("GPU texture readback failed"));
      return;
    }

    const QImage image = imageFromRhiReadback(rhi_, readbackResult);
    if (image.isNull()) {
      emitExportFailed(
          request.filePath,
          QStringLiteral("Failed to convert GPU readback to image"));
      return;
    }

    QImageWriter writer(request.filePath);
    if (!writer.write(image)) {
      const QString detail = writer.errorString();
      emitExportFailed(request.filePath,
                       detail.isEmpty()
                           ? QStringLiteral("Failed to write image file")
                           : detail);
      return;
    }

    emitExportSucceeded(request.filePath);
  }

  RhiToneMapper toneMapper_;
  RhiToneMapper exportToneMapper_;
  CpuAccumulatorDriver *driver_;
  QPointer<SystemView> viewPtr_;
  QThread *viewThread_ = nullptr;
  QRhi *rhi_ = nullptr;
  HistogramBuffer histogramCopy_;
  std::vector<ExportRequest> pendingExports_;
  uint64_t plottedIterations_ = 0;
  uint64_t lastSnapshotVersion_ = 0;
  float gamma_ = 2.2f;
  float exposure_ = 0.0f;
  float vibrancy_ = 0.0f;
  bool initialized_ = false;
  bool histogramDirty_ = true;
};

SystemView::SystemView(QQuickItem *parent) : QQuickRhiItem(parent) {
  driver_ = new CpuAccumulatorDriver(this);
  connect(driver_, &CpuAccumulatorDriver::batchCompleted, this,
          &SystemView::handleBatchCompleted);
  connect(driver_, &CpuAccumulatorDriver::errorOccurred, this,
          [this](const QString &message) {
            qCWarning(systemViewLog, "%s", qPrintable(message));
            if (running_) {
              running_ = false;
              emit runningChanged();
            }
            emit warningEmitted(message);
          });

  setColorBufferFormat(QQuickRhiItem::TextureFormat::RGBA8);
  setAlphaBlending(false);
  setMirrorVertically(false);
  setFixedColorBufferWidth(1);
  setFixedColorBufferHeight(1);
}

QQuickRhiItemRenderer *SystemView::createRenderer() {
  return new HistogramRenderer(this);
}

void SystemView::exportImage(const QString &filePath) {
  if (filePath.trimmed().isEmpty()) {
    emit imageExportFailed(filePath,
                           QStringLiteral("Export file path is empty"));
    return;
  }
  if (model_ == nullptr || model_->width() == 0 || model_->height() == 0) {
    emit imageExportFailed(filePath,
                           QStringLiteral("No project loaded to export"));
    return;
  }
  if (window() == nullptr) {
    emit imageExportFailed(
        filePath,
        QStringLiteral("Cannot export image without an active window"));
    return;
  }

  ExportRequest request;
  request.filePath = filePath;
  request.width = static_cast<uint32_t>(model_->width());
  request.height = static_cast<uint32_t>(model_->height());
  pendingExports_.push_back(std::move(request));
  update();
}

void SystemView::clear() {
  driver_->clear();
  resetConvergenceState();
  syncDriverRunning();
  update();
}

void SystemView::setRunning(bool running) {
  if (running_ == running) {
    return;
  }
  running_ = running;
  syncDriverRunning();
  emit runningChanged();
}

void SystemView::setTargetIterationsPerPixel(double targetIterationsPerPixel) {
  if (!std::isfinite(targetIterationsPerPixel) ||
      targetIterationsPerPixel <= 0.0 ||
      targetIterationsPerPixel == targetIterationsPerPixel_) {
    return;
  }
  targetIterationsPerPixel_ = targetIterationsPerPixel;
  emit targetIterationsPerPixelChanged();
  evaluateConvergence();
}

void SystemView::setInfiniteAccumulation(bool infiniteAccumulation) {
  if (infiniteAccumulation_ == infiniteAccumulation) {
    return;
  }
  infiniteAccumulation_ = infiniteAccumulation;
  emit infiniteAccumulationChanged();
  evaluateConvergence();
}

double SystemView::computeIterationsPerPixel(uint64_t plotted) const {
  const uint32_t w =
      model_ != nullptr ? (std::max)(1u, static_cast<uint32_t>(model_->width()))
                        : 1u;
  const uint32_t h =
      model_ != nullptr
          ? (std::max)(1u, static_cast<uint32_t>(model_->height()))
          : 1u;
  const double pixelCount = static_cast<double>(w) * static_cast<double>(h);
  return static_cast<double>(plotted) / pixelCount;
}

void SystemView::evaluateConvergence() {
  const bool nextConverged =
      !infiniteAccumulation_ &&
      currentIterationsPerPixel_ >= targetIterationsPerPixel_;
  if (nextConverged == converged_) {
    return;
  }
  converged_ = nextConverged;
  syncDriverRunning();
  emit convergedChanged();
  update();
}

void SystemView::resetConvergenceState() {
  const bool ippChanged =
      plottedIterations_ != 0 || currentIterationsPerPixel_ != 0.0;
  plottedIterations_ = 0;
  currentIterationsPerPixel_ = 0.0;
  if (ippChanged) {
    emit currentIterationsPerPixelChanged();
  }
  if (converged_) {
    converged_ = false;
    emit convergedChanged();
  }
}

void SystemView::syncDriverRunning() {
  driver_->setRunning(running_ && !converged_);
}

void SystemView::handleBatchCompleted() {
  if (!running_ || converged_) {
    return;
  }
  const uint64_t plotted = driver_->plottedIterations();
  const double ipp = computeIterationsPerPixel(plotted);
  if (plotted != plottedIterations_ || ipp != currentIterationsPerPixel_) {
    plottedIterations_ = plotted;
    currentIterationsPerPixel_ = ipp;
    emit currentIterationsPerPixelChanged();
  }
  evaluateConvergence();
  update();
}

void SystemView::setModel(ProjectModel *projectModel) {
  if (projectModel == model_) {
    return;
  }
  if (model_ != nullptr) {
    model_->disconnect(this);
    model_->system()->disconnect(this);
    model_->colorMap()->disconnect(this);
  }

  model_ = projectModel;
  if (model_ == nullptr) {
    driver_->clearSystem();
    updateBufferSize();
    emit modelChanged();
    update();
    return;
  }

  connect(model_, &ProjectModel::projectChanged, this,
          &SystemView::handleProjectChanged);
  connect(model_, &ProjectModel::gammaChanged, this, &QQuickItem::update);
  connect(model_, &ProjectModel::exposureChanged, this, &QQuickItem::update);
  connect(model_, &ProjectModel::vibrancyChanged, this, &QQuickItem::update);
  connect(model_, &ProjectModel::widthChanged, this,
          &SystemView::updateBufferSize);
  connect(model_, &ProjectModel::heightChanged, this,
          &SystemView::updateBufferSize);
  connect(model_->system(), &SystemModel::visuallyChanged, this,
          &SystemView::updateSystem);
  connect(model_->colorMap(), &ColorMapModel::nameChanged, this,
          &SystemView::updateColorMap);

  handleProjectChanged();
  emit modelChanged();
}

void SystemView::handleProjectChanged() {
  updateSystem();
  updateColorMap();
  updateBufferSize();
}

void SystemView::updateSystem() {
  if (model_ == nullptr) {
    return;
  }
  const System &systemProto = model_->proto()->system();

  std::vector<std::string> warnings;
  driver_->setSystem(toCameraSystem(systemProto, &warnings));
  for (const auto &warning : warnings) {
    qCWarning(systemViewLog, "%s", warning.c_str());
    emit warningEmitted(QString::fromStdString(warning));
  }
  resetConvergenceState();
  syncDriverRunning();
  update();
}

void SystemView::updateColorMap() {
  if (model_ == nullptr) {
    return;
  }
  driver_->setColorMap(model_->colorMap()->coreColorMap());
  resetConvergenceState();
  syncDriverRunning();
  update();
}

void SystemView::updateBufferSize() {
  const uint32_t w =
      model_ != nullptr ? (std::max)(1u, static_cast<uint32_t>(model_->width()))
                        : 1u;
  const uint32_t h =
      model_ != nullptr
          ? (std::max)(1u, static_cast<uint32_t>(model_->height()))
          : 1u;
  setFixedColorBufferWidth(static_cast<int>(w));
  setFixedColorBufferHeight(static_cast<int>(h));
  if (model_ != nullptr) {
    driver_->setSize(w, h);
  } else {
    driver_->setSize(0, 0);
  }
  resetConvergenceState();
  syncDriverRunning();
  update();
}

}  // namespace chaoskit::ui
