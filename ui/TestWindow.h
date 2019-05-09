#ifndef CHAOSKIT_UI_TESTWINDOW_H
#define CHAOSKIT_UI_TESTWINDOW_H

#include <QKeyEvent>
#include <QOpenGLWindow>
#include "GLToneMapper.h"
#include "HistogramGenerator.h"

namespace chaoskit::ui {

class TestWindow : public QOpenGLWindow {
 public:
  TestWindow();

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
