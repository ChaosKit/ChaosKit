#include "TestWindow.h"

namespace chaoskit {
namespace ui {

TestWindow::TestWindow() : buffer_(256, 256) {
  setTitle(QStringLiteral("ChaosKit (Test Window)"));
  setBaseSize(QSize(512, 512));

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(format);

  toneMapper_ = new GLToneMapper(this);

  *buffer_(127, 128) = {10000.f, 0.f, 0.f, 10000.f};
  *buffer_(128, 128) = {0.f, 10000.f, 0.f, 10000.f};
  *buffer_(129, 128) = {0.f, 0.f, 10000.f, 10000.f};
}

void TestWindow::initializeGL() {
  toneMapper_->initializeGL();
  toneMapper_->syncBuffer(buffer_);
}

void TestWindow::resizeGL(int w, int h) {
  const qreal pixelRatio = devicePixelRatio();
  auto *fns = context()->versionFunctions<QOpenGLFunctions_3_2_Core>();

  fns->glViewport(0, 0, static_cast<GLsizei>(w * pixelRatio),
                  static_cast<GLsizei>(h * pixelRatio));
}

void TestWindow::paintGL() { toneMapper_->map(); }

}  // namespace ui
}  // namespace chaoskit
