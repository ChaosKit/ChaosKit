#ifndef CHAOSKIT_UI_SYSTEMVIEW_H
#define CHAOSKIT_UI_SYSTEMVIEW_H

#include <QQuickRhiItem>
#include <QString>
#include <QtGlobal>
#include <cstdint>

#include "CpuAccumulatorDriver.h"
#include "models/ProjectModel.h"

namespace chaoskit::ui {

class SystemView : public QQuickRhiItem {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::ProjectModel *model READ model WRITE setModel NOTIFY
                 modelChanged)
  Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
  Q_PROPERTY(
      double targetIterationsPerPixel READ targetIterationsPerPixel WRITE
          setTargetIterationsPerPixel NOTIFY targetIterationsPerPixelChanged)
  Q_PROPERTY(bool infiniteAccumulation READ infiniteAccumulation WRITE
                 setInfiniteAccumulation NOTIFY infiniteAccumulationChanged)
  Q_PROPERTY(double currentIterationsPerPixel READ currentIterationsPerPixel
                 NOTIFY currentIterationsPerPixelChanged)
  Q_PROPERTY(bool converged READ converged NOTIFY convergedChanged)
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
  void warningEmitted(const QString &warning);

 private:
  [[nodiscard]] double computeIterationsPerPixel(uint64_t plotted) const;
  void evaluateConvergence();
  void resetConvergenceState();
  void syncDriverRunning();

  CpuAccumulatorDriver *driver_;
  ProjectModel *model_ = nullptr;
  double targetIterationsPerPixel_ = 1000.0;
  bool infiniteAccumulation_ = false;
  double currentIterationsPerPixel_ = 0.0;
  bool converged_ = false;
  bool running_ = false;
  uint64_t plottedIterations_ = 0;

 private slots:
  void handleBatchCompleted();
  void handleProjectChanged();
  void updateColorMap();
  void updateSystem();
  void updateBufferSize();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_SYSTEMVIEW_H
