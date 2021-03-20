#ifndef CHAOSKIT_UI_SYSTEMVIEW_H
#define CHAOSKIT_UI_SYSTEMVIEW_H

#include <QQuickFramebufferObject>
#include "HistogramGenerator.h"
#include "models/ColorMapRegistry.h"
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

  void synchronizeResult(core::Renderer *renderer) const {
    generator_->synchronizeResult(renderer);
  }

  [[nodiscard]] ProjectModel *model() const { return model_; }

  [[nodiscard]] bool running() const { return generator_->isEnabled(); }

 public slots:
  void clear();
  void setRunning(bool running);
  void setModel(ProjectModel *projectModel);

 signals:
  void runningChanged();
  void modelChanged();

 private:
  HistogramGenerator *generator_;
  ProjectModel *model_ = nullptr;

 private slots:
  void updateColorMap();
  void updateSystem();
  void updateBufferSize();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_SYSTEMVIEW_H
