#ifndef CHAOSKIT_UI_SYSTEMVIEW_H
#define CHAOSKIT_UI_SYSTEMVIEW_H

#include <QQuickFramebufferObject>
#include <QString>
#include <QtGlobal>
#include "CpuAccumulatorDriver.h"
#include "models/ProjectModel.h"

namespace chaoskit::ui {

class SystemView : public QQuickFramebufferObject {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::ProjectModel *model READ model WRITE setModel NOTIFY
                 modelChanged)
  Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
 public:
  explicit SystemView(QQuickItem *parent = nullptr);

  [[nodiscard]] Renderer *createRenderer() const override;

  [[nodiscard]] CpuAccumulatorDriver *driver() const { return driver_; }
  [[nodiscard]] ProjectModel *model() const { return model_; }
  [[nodiscard]] bool running() const { return driver_->isRunning(); }
  [[nodiscard]] quint64 plottedIterations() const {
    return static_cast<quint64>(driver_->plottedIterations());
  }

 public slots:
  void clear();
  void setRunning(bool running);
  void setModel(ProjectModel *projectModel);

 signals:
  void runningChanged();
  void modelChanged();
  void warningEmitted(const QString &warning);

 private:
  CpuAccumulatorDriver *driver_;
  ProjectModel *model_ = nullptr;

 private slots:
  void updateColorMap();
  void updateSystem();
  void updateBufferSize();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_SYSTEMVIEW_H
