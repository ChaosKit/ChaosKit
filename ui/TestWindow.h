#ifndef CHAOSKIT_UI_TESTWINDOW_H
#define CHAOSKIT_UI_TESTWINDOW_H

#include <QOpenGLWindow>
#include <core/HistogramBuffer.h>
#include "GLToneMapper.h"

namespace chaoskit {
namespace ui {

class TestWindow : public QOpenGLWindow {
 public:
  TestWindow();

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

 private:
  core::HistogramBuffer buffer_;
  GLToneMapper *toneMapper_;
};

}
}

#endif //CHAOSKIT_UI_TESTWINDOW_H
