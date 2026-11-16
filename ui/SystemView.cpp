#include "SystemView.h"

#include <rhi/qrhi.h>
#include <QLoggingCategory>
#include <QQuickItem>
#include <QString>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "RhiToneMapper.h"
#include "core/HistogramBuffer.h"
#include "ui/storage/conversions.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit::ui {

Q_LOGGING_CATEGORY(systemViewLog, "SystemView");

namespace {

class HistogramRenderer : public QQuickRhiItemRenderer {
 public:
  explicit HistogramRenderer(const SystemView *view)
      : driver_(view->driver()) {}
  ~HistogramRenderer() override { toneMapper_.releaseResources(); }

 protected:
  void initialize(QRhiCommandBuffer * /*cb*/) override {
    if (rhi_ != rhi()) {
      toneMapper_.releaseResources();
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
      toneMapper_.setGamma(model->gamma());
      toneMapper_.setExposure(model->exposure());
      toneMapper_.setVibrancy(model->vibrancy());
    }

    if (driver_->syncSnapshotIfChanged(lastSnapshotVersion_, histogramCopy_,
                                       &plottedIterations_)) {
      histogramDirty_ = true;
    }
  }

  void render(QRhiCommandBuffer *cb) override {
    if (!initialized_ || renderTarget() == nullptr) {
      return;
    }

    toneMapper_.render(cb, renderTarget(),
                       histogramDirty_ ? &histogramCopy_ : nullptr,
                       plottedIterations_);
    histogramDirty_ = false;
  }

 private:
  RhiToneMapper toneMapper_;
  CpuAccumulatorDriver *driver_;
  QRhi *rhi_ = nullptr;
  HistogramBuffer histogramCopy_;
  uint64_t plottedIterations_ = 0;
  uint64_t lastSnapshotVersion_ = 0;
  bool initialized_ = false;
  bool histogramDirty_ = true;
};

}  // namespace

SystemView::SystemView(QQuickItem *parent) : QQuickRhiItem(parent) {
  driver_ = new CpuAccumulatorDriver(this);
  connect(driver_, &CpuAccumulatorDriver::batchCompleted, this,
          &QQuickItem::update);
  connect(driver_, &CpuAccumulatorDriver::runningChanged, this,
          &SystemView::runningChanged);
  connect(driver_, &CpuAccumulatorDriver::errorOccurred, this,
          [this](const QString &message) {
            qCWarning(systemViewLog, "%s", qPrintable(message));
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

void SystemView::clear() {
  driver_->clear();
  update();
}

void SystemView::setRunning(bool running) { driver_->setRunning(running); }

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

  updateSystem();
  updateColorMap();
  updateBufferSize();
  emit modelChanged();
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
  update();
}

void SystemView::updateColorMap() {
  if (model_ == nullptr) {
    return;
  }
  driver_->setColorMap(model_->colorMap()->coreColorMap());
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
  update();
}

}  // namespace chaoskit::ui
