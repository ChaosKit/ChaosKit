#ifndef CHAOSKIT_UI_SYSTEMVIEW_H
#define CHAOSKIT_UI_SYSTEMVIEW_H

#include <QQuickRhiItem>
#include <QSize>
#include <QString>
#include <QtGlobal>

#include <cstdint>
#include <optional>
#include <vector>

#include "CpuAccumulatorDriver.h"
#include "core/CameraSystem.h"
#include "core/ColorMap.h"
#include "models/ProjectModel.h"

namespace chaoskit::ui {

class SystemView : public QQuickRhiItem {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::ProjectModel *model READ model WRITE setModel NOTIFY
                 modelChanged)
  Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
  Q_PROPERTY(double targetIterationsPerPixel READ targetIterationsPerPixel WRITE
                 setTargetIterationsPerPixel NOTIFY
                     targetIterationsPerPixelChanged FINAL)
  Q_PROPERTY(
      bool infiniteAccumulation READ infiniteAccumulation WRITE
          setInfiniteAccumulation NOTIFY infiniteAccumulationChanged FINAL)
  Q_PROPERTY(double currentIterationsPerPixel READ currentIterationsPerPixel
                 NOTIFY currentIterationsPerPixelChanged FINAL)
  Q_PROPERTY(bool converged READ converged NOTIFY convergedChanged FINAL)
  Q_PROPERTY(quint64 compiledPipelineCount READ compiledPipelineCount NOTIFY
                 compiledPipelineCountChanged FINAL)
  Q_PROPERTY(bool gpuComputeActive READ gpuComputeActive NOTIFY
                 gpuComputeActiveChanged FINAL)
 public:
  explicit SystemView(QQuickItem *parent = nullptr);

  [[nodiscard]] QQuickRhiItemRenderer *createRenderer() override;

  [[nodiscard]] CpuAccumulatorDriver *driver() const { return driver_; }
  [[nodiscard]] ProjectModel *model() const { return model_; }
  [[nodiscard]] bool running() const { return running_; }
  [[nodiscard]] double targetIterationsPerPixel() const {
    return targetIterationsPerPixel_;
  }
  [[nodiscard]] bool infiniteAccumulation() const {
    return infiniteAccumulation_;
  }
  [[nodiscard]] double currentIterationsPerPixel() const {
    return currentIterationsPerPixel_;
  }
  [[nodiscard]] bool converged() const { return converged_; }
  [[nodiscard]] quint64 plottedIterations() const {
    return static_cast<quint64>(plottedIterations_);
  }
  [[nodiscard]] quint64 compiledPipelineCount() const {
    return compiledPipelineCount_;
  }
  [[nodiscard]] bool gpuComputeActive() const { return gpuComputeActive_; }

  Q_INVOKABLE void exportImage(const QString &filePath);

 public slots:
  void clear();
  void setRunning(bool running);
  void setModel(ProjectModel *projectModel);
  void setTargetIterationsPerPixel(double targetIterationsPerPixel);
  void setInfiniteAccumulation(bool infiniteAccumulation);

 signals:
  void runningChanged();
  void modelChanged();
  void targetIterationsPerPixelChanged();
  void infiniteAccumulationChanged();
  void currentIterationsPerPixelChanged();
  void convergedChanged();
  void compiledPipelineCountChanged();
  void gpuComputeActiveChanged();
  void warningEmitted(const QString &warning);
  void imageExportSucceeded(const QString &filePath);
  void imageExportFailed(const QString &filePath, const QString &error);

 private slots:
  void handleBatchCompleted();
  void handleProjectChanged();
  void updateColorMap();
  void updateSystem();
  void updateBufferSize();

 private:
  class HistogramRenderer;

  struct ExportRequest {
    QString filePath;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  [[nodiscard]] double computeIterationsPerPixel(uint64_t plotted) const;
  void evaluateConvergence();
  void resetConvergenceState();
  void syncDriverRunning();
  void handleRendererBackendReady(bool gpuActive);
  void handleGpuCompileCountChanged(quint64 compileCount);
  void handleGpuReadback(uint64_t generation, uint64_t plottedIterations);
  void handleGpuError(const QString &message);

  CpuAccumulatorDriver *driver_;
  ProjectModel *model_ = nullptr;
  double targetIterationsPerPixel_ = 1000.0;
  bool infiniteAccumulation_ = false;
  double currentIterationsPerPixel_ = 0.0;
  bool converged_ = false;
  bool running_ = false;
  bool backendKnown_ = false;
  bool gpuComputeActive_ = false;
  uint64_t plottedIterations_ = 0;
  quint64 compiledPipelineCount_ = 0;
  std::optional<core::CameraSystem> gpuSystem_ = std::nullopt;
  const core::ColorMap *colorMapPtr_ = nullptr;
  QSize bufferSize_ = QSize(1, 1);
  uint64_t systemVersion_ = 0;
  uint64_t colorMapVersion_ = 0;
  uint64_t sizeVersion_ = 0;
  uint64_t editGeneration_ = 1;
  std::vector<ExportRequest> pendingExports_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_SYSTEMVIEW_H
