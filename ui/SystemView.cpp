#include "SystemView.h"

#include <rhi/qrhi.h>
#include <QAbstractEventDispatcher>
#include <QImage>
#include <QImageWriter>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QSize>
#include <QString>
#include <QThread>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <ratio>
#include <string>
#include <utility>
#include <vector>

#include "RhiComputeEngine.h"
#include "RhiOffscreenContext.h"
#include "RhiToneMapper.h"
#include "core/CameraSystem.h"
#include "core/ColorMap.h"
#include "core/HistogramBuffer.h"
#include "ui/storage/conversions.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit::ui {

namespace {

constexpr uint32_t kGpuParticleCount = 4096;
constexpr uint32_t kBootstrapTicks = 4;
constexpr uint32_t kMinTicksPerInvocation = 1;
constexpr uint32_t kMaxTicksPerInvocation = 2048;
constexpr double kTargetLowMs = 8.0;
constexpr double kTargetHighMs = 12.0;
constexpr double kTargetMidMs = 10.0;
constexpr double kIdleGapThresholdMs = 250.0;
constexpr double kEmaAlpha = 0.3;

}  // namespace

Q_LOGGING_CATEGORY(systemViewLog, "SystemView");

class SystemView::HistogramRenderer : public QQuickRhiItemRenderer {
 public:
  explicit HistogramRenderer(SystemView *view)
      : driver_(view->driver()),
        viewPtr_(view),
        viewThread_(view->thread()),
        aliveToken_(std::make_shared<bool>(true)) {}

  ~HistogramRenderer() override {
    failPendingExports(
        QStringLiteral("Renderer destroyed before export completed"));
    if (aliveToken_) {
      *aliveToken_ = false;
      aliveToken_.reset();
    }
    if (rhi_ != nullptr && !rhi_->isRecordingFrame() && readbackInFlight_) {
      rhi_->finish();
    }
    computeEngine_.releaseResources();
    toneMapper_.releaseResources();
    exportToneMapper_.releaseResources();
  }

 protected:
  void initialize(QRhiCommandBuffer * /*cb*/) override {
    if (rhi_ != rhi()) {
      if (rhi_ != nullptr && !rhi_->isRecordingFrame() && readbackInFlight_) {
        rhi_->finish();
      }
      if (aliveToken_) {
        *aliveToken_ = false;
      }
      aliveToken_ = std::make_shared<bool>(true);
      readbackInFlight_ = false;

      computeEngine_.releaseResources();
      toneMapper_.releaseResources();
      exportToneMapper_.releaseResources();
      rhi_ = rhi();
      initialized_ = false;
      useGpuCompute_ = false;
      computeInitialized_ = false;
      histogramDirty_ = true;
      lastToneMapperHistogram_ = nullptr;
      lastComputeResourceEpoch_ = 0;

      if (activeSystem_.has_value() || pendingSystem_.has_value()) {
        pendingSystemUpdate_ = true;
      }
      pendingColorMapUpdate_ = true;
      pendingSizeUpdate_ = true;
      pendingClearReset_ = true;
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

    if (!initialized_) {
      bool gpuComputeSupported = rhi_->isFeatureSupported(QRhi::Compute);
      if (gpuComputeSupported) {
        QString computeError;
        if (computeEngine_.initialize(rhi_, &computeError)) {
          computeEngine_.setParticleCount(kGpuParticleCount);
          computeInitialized_ = true;
          useGpuCompute_ = true;
        } else {
          qCWarning(systemViewLog,
                    "Failed to initialize RhiComputeEngine, falling back to "
                    "CPU: %s",
                    qPrintable(computeError));
          computeEngine_.releaseResources();
          computeInitialized_ = false;
          useGpuCompute_ = false;
        }
      } else {
        useGpuCompute_ = false;
      }

      const bool gpuActive = useGpuCompute_;
      invokeOnViewThread([gpuActive](SystemView *view) {
        view->handleRendererBackendReady(gpuActive);
      });
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

    running_ = view->running();
    converged_ = view->converged();
    infiniteAccumulation_ = view->infiniteAccumulation();
    targetIterationsPerPixel_ = view->targetIterationsPerPixel();
    knownPlottedIterations_ = view->plottedIterations();

    if (const QQuickWindow *win = view->window()) {
      if (const QScreen *screen = win->screen()) {
        const qreal hz = screen->refreshRate();
        if (hz > 1.0) {
          screenRefreshHz_ = static_cast<double>(hz);
        }
      }
    }

    if (lastSyncedSizeVersion_ != view->sizeVersion_) {
      targetBufferSize_ = view->bufferSize_;
      pendingSizeUpdate_ = true;
      lastSyncedSizeVersion_ = view->sizeVersion_;
    }
    if (lastSyncedColorMapVersion_ != view->colorMapVersion_) {
      pendingColorMap_ = view->colorMapPtr_;
      pendingColorMapUpdate_ = true;
      lastSyncedColorMapVersion_ = view->colorMapVersion_;
    }
    if (lastSyncedSystemVersion_ != view->systemVersion_) {
      pendingSystem_ = view->gpuSystem_;
      pendingSystemUpdate_ = true;
      lastSyncedSystemVersion_ = view->systemVersion_;
    }
    if (lastSyncedClearGeneration_ != view->editGeneration_) {
      currentGeneration_ = view->editGeneration_;
      pendingClearReset_ = true;
      knownPlottedIterations_ = 0;
      lastSyncedClearGeneration_ = view->editGeneration_;
    }

    if (!useGpuCompute_) {
      if (driver_->syncSnapshotIfChanged(lastSnapshotVersion_, histogramCopy_,
                                         &plottedIterations_)) {
        histogramDirty_ = true;
      }
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

    if (useGpuCompute_ && computeInitialized_) {
      renderGpuCompute(cb);
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
  struct CounterReadback {
    QRhiReadbackResult result;
    uint64_t generation = 0;
    uint64_t readbackId = 0;
    std::shared_ptr<CounterReadback> self;
  };

  void resetDispatchPacing() {
    ticksPerInvocation_ = kBootstrapTicks;
    hasEmaFrameMs_ = false;
    emaFrameMs_ = 0.0;
    hasLastActiveFrame_ = false;
    overloadCapTicks_ = kMaxTicksPerInvocation;
    estimatedFallbackPlotted_ = 0;
  }

  uint32_t nextDispatchTicks() {
    const auto now = std::chrono::steady_clock::now();
    if (hasLastActiveFrame_) {
      const double elapsedMs = (now - lastActiveFrameTime_) /
                               std::chrono::duration<double, std::milli>(1.0);
      if (elapsedMs > 0.0 && elapsedMs < kIdleGapThresholdMs) {
        emaFrameMs_ =
            hasEmaFrameMs_
                ? (kEmaAlpha * elapsedMs + (1.0 - kEmaAlpha) * emaFrameMs_)
                : elapsedMs;
        hasEmaFrameMs_ = true;

        const double vsyncPeriodMs =
            (screenRefreshHz_ > 1.0) ? (1000.0 / screenRefreshHz_) : 16.6667;
        const bool isVsyncPinned =
            (vsyncPeriodMs >= 13.0) && (emaFrameMs_ <= vsyncPeriodMs * 1.15);

        if (emaFrameMs_ > (std::max)(kTargetHighMs, vsyncPeriodMs * 1.2)) {
          overloadCapTicks_ =
              (std::max)(kMinTicksPerInvocation,
                         static_cast<uint32_t>(
                             static_cast<double>(ticksPerInvocation_) * 0.85));
          const double scale =
              std::clamp(kTargetMidMs / emaFrameMs_, 0.5, 0.85);
          ticksPerInvocation_ =
              std::clamp(static_cast<uint32_t>(
                             static_cast<double>(ticksPerInvocation_) * scale),
                         kMinTicksPerInvocation, kMaxTicksPerInvocation);
        } else if (emaFrameMs_ < kTargetLowMs) {
          const double scale = std::clamp(
              kTargetMidMs / (std::max)(1.0, emaFrameMs_), 1.15, 1.75);
          const uint32_t nextTicks =
              (std::max)(ticksPerInvocation_ + 1u,
                         static_cast<uint32_t>(
                             static_cast<double>(ticksPerInvocation_) * scale));
          ticksPerInvocation_ = (std::min)(nextTicks, overloadCapTicks_);
        } else if (isVsyncPinned) {
          const uint32_t nextTicks =
              (std::max)(ticksPerInvocation_ + 1u,
                         static_cast<uint32_t>(
                             static_cast<double>(ticksPerInvocation_) * 1.35));
          ticksPerInvocation_ = (std::min)(nextTicks, overloadCapTicks_);
        }
      }
    }
    lastActiveFrameTime_ = now;
    hasLastActiveFrame_ = true;

    uint32_t dispatchTicks = ticksPerInvocation_;
    if (!infiniteAccumulation_ && targetIterationsPerPixel_ > 0.0) {
      const uint64_t pixelCount =
          static_cast<uint64_t>((std::max)(1, targetBufferSize_.width())) *
          static_cast<uint64_t>((std::max)(1, targetBufferSize_.height()));
      const uint64_t targetTotal = static_cast<uint64_t>(std::ceil(
          targetIterationsPerPixel_ * static_cast<double>(pixelCount)));
      const uint64_t remaining = (targetTotal > knownPlottedIterations_)
                                     ? (targetTotal - knownPlottedIterations_)
                                     : 1u;
      const uint32_t particleCount = computeEngine_.particleCount();
      const uint64_t warmupMargin =
          (knownPlottedIterations_ == 0 && activeSystem_.has_value())
              ? static_cast<uint64_t>(
                    (std::max)(0, activeSystem_->lifetime.min()))
              : 0u;
      const uint64_t maxNeeded =
          (remaining + particleCount - 1u) / particleCount + warmupMargin;
      const uint32_t capped = static_cast<uint32_t>(
          (std::min)(maxNeeded, static_cast<uint64_t>(kMaxTicksPerInvocation)));
      dispatchTicks = std::clamp(dispatchTicks, kMinTicksPerInvocation,
                                 (std::max)(kMinTicksPerInvocation, capped));
    }
    return dispatchTicks;
  }

  void enqueueCounterReadback(QRhiCommandBuffer *cb, uint32_t ticksDispatched) {
    if (!rhi_->isFeatureSupported(QRhi::ReadBackNonUniformBuffer)) {
      estimatedFallbackPlotted_ +=
          static_cast<uint64_t>(computeEngine_.particleCount()) *
          ticksDispatched;
      const uint64_t gen = currentGeneration_;
      const uint64_t plotted = estimatedFallbackPlotted_;
      knownPlottedIterations_ = plotted;
      invokeOnViewThread([gen, plotted](SystemView *view) {
        view->handleGpuReadback(gen, plotted);
      });
      return;
    }

    if (readbackInFlight_ && inFlightGeneration_ == currentGeneration_) {
      return;
    }

    const uint64_t readbackGen = currentGeneration_;
    const uint64_t readbackId = ++latestReadbackId_;
    inFlightGeneration_ = readbackGen;
    readbackInFlight_ = true;

    auto req = std::make_shared<CounterReadback>();
    req->generation = readbackGen;
    req->readbackId = readbackId;
    req->self = req;

    std::weak_ptr<CounterReadback> weakReq = req;
    std::weak_ptr<bool> weakAlive = aliveToken_;
    HistogramRenderer *renderer = this;

    req->result.completed = [renderer, weakAlive, weakReq]() {
      std::shared_ptr<CounterReadback> lockedReq = weakReq.lock();
      if (!lockedReq) {
        return;
      }
      // Break self-ownership immediately so req is freed when callback exits.
      std::shared_ptr<CounterReadback> selfRef = std::move(lockedReq->self);

      std::shared_ptr<bool> alive = weakAlive.lock();
      if (!alive || !*alive) {
        return;
      }

      if (lockedReq->readbackId == renderer->latestReadbackId_) {
        renderer->readbackInFlight_ = false;
      }
      if (lockedReq->generation != renderer->currentGeneration_ ||
          lockedReq->result.data.size() < 8) {
        return;
      }

      uint32_t counterWords[2] = {0, 0};
      std::memcpy(counterWords, lockedReq->result.data.constData(),
                  sizeof(counterWords));
      const uint64_t plotted = (static_cast<uint64_t>(counterWords[1]) << 32u) |
                               static_cast<uint64_t>(counterWords[0]);

      renderer->knownPlottedIterations_ = plotted;
      const uint64_t gen = lockedReq->generation;
      renderer->invokeOnViewThread([gen, plotted](SystemView *view) {
        view->handleGpuReadback(gen, plotted);
      });
    };

    QRhiResourceUpdateBatch *counterBatch = rhi_->nextResourceUpdateBatch();
    counterBatch->readBackBuffer(computeEngine_.counterBuffer(), 0, 16,
                                 &req->result);
    cb->resourceUpdate(counterBatch);
  }

  void renderGpuCompute(QRhiCommandBuffer *cb) {
    if (pendingSizeUpdate_) {
      computeEngine_.setSize(targetBufferSize_);
      pendingSizeUpdate_ = false;
    }
    if (pendingColorMapUpdate_) {
      computeEngine_.setColorMap(pendingColorMap_);
      pendingColorMapUpdate_ = false;
    }
    if (pendingSystemUpdate_) {
      if (pendingSystem_.has_value()) {
        QString compileError;
        if (!computeEngine_.setSystem(*pendingSystem_, &compileError)) {
          hasValidSystem_ = false;
          activeSystem_.reset();
          computeEngine_.clearSystem();
          invokeOnViewThread([compileError](SystemView *view) {
            view->handleGpuError(compileError);
          });
        } else {
          hasValidSystem_ = true;
          activeSystem_ = pendingSystem_;
        }
      } else {
        hasValidSystem_ = false;
        activeSystem_.reset();
        computeEngine_.clearSystem();
      }
      pendingSystemUpdate_ = false;
    }
    if (pendingClearReset_) {
      computeEngine_.reset();
      resetDispatchPacing();
      pendingClearReset_ = false;
    }

    const uint64_t compiles = computeEngine_.compileCount();
    if (compiles != lastReportedCompileCount_) {
      lastReportedCompileCount_ = compiles;
      invokeOnViewThread([compiles](SystemView *view) {
        view->handleGpuCompileCountChanged(compiles);
      });
    }

    const bool shouldAccumulate = running_ && !converged_ && hasValidSystem_ &&
                                  !targetBufferSize_.isEmpty();
    if (shouldAccumulate) {
      const uint32_t ticks = nextDispatchTicks();
      QString dispatchError;
      if (!computeEngine_.dispatch(cb, ticks, &dispatchError)) {
        hasValidSystem_ = false;
        invokeOnViewThread([dispatchError](SystemView *view) {
          view->handleGpuError(dispatchError);
        });
      } else {
        enqueueCounterReadback(cb, ticks);
      }
    } else {
      hasLastActiveFrame_ = false;
      QString prepError;
      if (!computeEngine_.prepareAndClear(cb, &prepError)) {
        qCWarning(systemViewLog, "Failed to prepare compute buffers: %s",
                  qPrintable(prepError));
      }
    }

    QRhiBuffer *histBuf = computeEngine_.histogramBuffer();
    if (computeEngine_.resourceEpoch() != lastComputeResourceEpoch_ &&
        histBuf != nullptr && histBuf == lastToneMapperHistogram_) {
      toneMapper_.releaseResources();
      QString reinitError;
      if (!toneMapper_.initialize(rhi_, renderTarget()->renderPassDescriptor(),
                                  renderTarget()->sampleCount(),
                                  &reinitError)) {
        qCWarning(systemViewLog, "Failed to reinitialize RhiToneMapper: %s",
                  qPrintable(reinitError));
        failPendingExports(
            QStringLiteral("GPU renderer failed to reinitialize"));
        return;
      }
    }
    lastComputeResourceEpoch_ = computeEngine_.resourceEpoch();
    lastToneMapperHistogram_ = histBuf;

    toneMapper_.renderFromSsbo(cb, renderTarget(), histBuf,
                               computeEngine_.counterBuffer(),
                               computeEngine_.size());

    if (!pendingExports_.empty()) {
      std::vector<ExportRequest> exports = std::move(pendingExports_);
      pendingExports_.clear();
      for (const ExportRequest &request : exports) {
        executeExport(cb, request);
      }
    }

    if (shouldAccumulate) {
      update();
    }
  }

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

    if (useGpuCompute_ && computeInitialized_ &&
        computeEngine_.histogramBuffer() != nullptr &&
        computeEngine_.counterBuffer() != nullptr) {
      exportToneMapper_.renderFromSsbo(
          cb, exportTarget.get(), computeEngine_.histogramBuffer(),
          computeEngine_.counterBuffer(), computeEngine_.size(), readbackBatch);
    } else {
      const HistogramBuffer *histogramToUpload =
          (histogramCopy_.width() > 0 && histogramCopy_.height() > 0)
              ? &histogramCopy_
              : nullptr;
      exportToneMapper_.render(cb, exportTarget.get(), histogramToUpload,
                               plottedIterations_, readbackBatch);
    }

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

  RhiComputeEngine computeEngine_;
  RhiToneMapper toneMapper_;
  RhiToneMapper exportToneMapper_;
  CpuAccumulatorDriver *driver_;
  QPointer<SystemView> viewPtr_;
  QThread *viewThread_ = nullptr;
  std::shared_ptr<bool> aliveToken_;
  QRhi *rhi_ = nullptr;
  QRhiBuffer *lastToneMapperHistogram_ = nullptr;
  uint64_t lastComputeResourceEpoch_ = 0;

  std::optional<core::CameraSystem> activeSystem_ = std::nullopt;
  std::optional<core::CameraSystem> pendingSystem_ = std::nullopt;
  const core::ColorMap *pendingColorMap_ = nullptr;
  QSize targetBufferSize_ = QSize(1, 1);

  uint64_t lastSyncedSystemVersion_ = 0;
  uint64_t lastSyncedColorMapVersion_ = 0;
  uint64_t lastSyncedSizeVersion_ = 0;
  uint64_t lastSyncedClearGeneration_ = 0;
  uint64_t currentGeneration_ = 1;
  uint64_t lastReportedCompileCount_ = 0;

  bool pendingSystemUpdate_ = false;
  bool pendingColorMapUpdate_ = false;
  bool pendingSizeUpdate_ = false;
  bool pendingClearReset_ = false;
  bool hasValidSystem_ = false;

  bool running_ = false;
  bool converged_ = false;
  bool infiniteAccumulation_ = false;
  double targetIterationsPerPixel_ = 1000.0;
  uint64_t knownPlottedIterations_ = 0;
  double screenRefreshHz_ = 60.0;

  uint32_t ticksPerInvocation_ = kBootstrapTicks;
  uint32_t overloadCapTicks_ = kMaxTicksPerInvocation;
  bool hasEmaFrameMs_ = false;
  double emaFrameMs_ = 0.0;
  bool hasLastActiveFrame_ = false;
  std::chrono::steady_clock::time_point lastActiveFrameTime_ = {};
  uint64_t estimatedFallbackPlotted_ = 0;

  bool readbackInFlight_ = false;
  uint64_t inFlightGeneration_ = 0;
  uint64_t latestReadbackId_ = 0;

  HistogramBuffer histogramCopy_;
  std::vector<ExportRequest> pendingExports_;
  uint64_t plottedIterations_ = 0;
  uint64_t lastSnapshotVersion_ = 0;
  float gamma_ = 2.2f;
  float exposure_ = 0.0f;
  float vibrancy_ = 0.0f;
  bool initialized_ = false;
  bool useGpuCompute_ = false;
  bool computeInitialized_ = false;
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
  ++editGeneration_;
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
  if (running_ && !converged_) {
    update();
  }
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
  driver_->setRunning(backendKnown_ && !gpuComputeActive_ && running_ &&
                      !converged_);
}

void SystemView::handleRendererBackendReady(bool gpuActive) {
  backendKnown_ = true;
  if (gpuComputeActive_ != gpuActive) {
    gpuComputeActive_ = gpuActive;
    if (gpuComputeActive_) {
      driver_->setRunning(false);
      driver_->clear();
    }
    emit gpuComputeActiveChanged();
  }
  syncDriverRunning();
  if (running_ && !converged_) {
    update();
  }
}

void SystemView::handleGpuCompileCountChanged(quint64 compileCount) {
  if (compiledPipelineCount_ == compileCount) {
    return;
  }
  compiledPipelineCount_ = compileCount;
  emit compiledPipelineCountChanged();
}

void SystemView::handleGpuReadback(uint64_t generation,
                                   uint64_t plottedIterations) {
  if (!gpuComputeActive_ || generation != editGeneration_) {
    return;
  }
  if (!running_ || converged_) {
    return;
  }
  const double ipp = computeIterationsPerPixel(plottedIterations);
  if (plottedIterations != plottedIterations_ ||
      ipp != currentIterationsPerPixel_) {
    plottedIterations_ = plottedIterations;
    currentIterationsPerPixel_ = ipp;
    emit currentIterationsPerPixelChanged();
  }
  evaluateConvergence();
}

void SystemView::handleGpuError(const QString &message) {
  qCWarning(systemViewLog, "%s", qPrintable(message));
  if (running_) {
    running_ = false;
    syncDriverRunning();
    emit runningChanged();
  }
  emit warningEmitted(message);
}

void SystemView::handleBatchCompleted() {
  if (gpuComputeActive_ || !running_ || converged_) {
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
    gpuSystem_.reset();
    ++systemVersion_;
    colorMapPtr_ = nullptr;
    ++colorMapVersion_;
    ++editGeneration_;
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
  core::CameraSystem cameraSystem = toCameraSystem(systemProto, &warnings);
  for (const auto &warning : warnings) {
    qCWarning(systemViewLog, "%s", warning.c_str());
    emit warningEmitted(QString::fromStdString(warning));
  }

  gpuSystem_ = cameraSystem;
  ++systemVersion_;
  ++editGeneration_;
  driver_->setSystem(cameraSystem);
  resetConvergenceState();
  syncDriverRunning();
  update();
}

void SystemView::updateColorMap() {
  if (model_ == nullptr) {
    return;
  }
  colorMapPtr_ = model_->colorMap()->coreColorMap();
  ++colorMapVersion_;
  ++editGeneration_;
  driver_->setColorMap(colorMapPtr_);
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
  bufferSize_ = QSize(static_cast<int>(w), static_cast<int>(h));
  ++sizeVersion_;
  ++editGeneration_;
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
