#ifndef CHAOSKIT_UI_TESTWINDOW_H
#define CHAOSKIT_UI_TESTWINDOW_H

#include <QKeyEvent>
#include <QOpenGLWindow>
#include "GLToneMapper.h"
#include "HistogramGenerator.h"
#include "core/Renderer.h"

namespace chaoskit::ui {

class TestWindow : public QOpenGLWindow, public core::Renderer {
 public:
  TestWindow();
  void updateHistogramBuffer(const core::HistogramBuffer &buffer) override;

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void keyPressEvent(QKeyEvent *event) override;

 private:
  HistogramGenerator *histogramGenerator_;
  GLToneMapper *toneMapper_;

 private slots:
  void syncHistogram();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_TESTWINDOW_H
