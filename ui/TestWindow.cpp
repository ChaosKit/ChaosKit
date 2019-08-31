#include "TestWindow.h"
#include <library/FormulaType.h>
#include <QTimer>
#include "DocumentModel.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::library::FormulaType;

namespace chaoskit::ui {

TestWindow::TestWindow() {
  setTitle(QStringLiteral("ChaosKit (Test Window)"));
  setBaseSize(QSize(512, 512));

  auto *model = new DocumentModel(this);
  QModelIndex blendIndex = model->addBlend();
  QModelIndex formulaIndex = model->addFormula(FormulaType::DeJong, blendIndex);
  model->setData(formulaIndex,
                 QVariant::fromValue(std::vector<float>{
                     9.379666578024626e-01f, 1.938709271140397e+00f,
                     -1.580897020176053e-01f, -1.430070123635232e+00f}),
                 DocumentModel::ParamsRole);
  model->setData(
      model->finalBlendIndex(),
      QVariant::fromValue(QTransform::fromScale(.5, 1).translate(.5, .5)),
      DocumentModel::PostTransformRole);

  histogramGenerator_ = new HistogramGenerator(this);
  histogramGenerator_->setSize(512, 512);
  histogramGenerator_->setSystem(model->document()->system);
  histogramGenerator_->setTtl(20);
  toneMapper_ = new GLToneMapper(this);

  connect(this, &QOpenGLWindow::frameSwapped, this, &TestWindow::syncHistogram);
}

void TestWindow::initializeGL() {
  toneMapper_->initializeGL();
  histogramGenerator_->start();
}

void TestWindow::resizeGL(int w, int h) {
  const qreal pixelRatio = devicePixelRatio();
  auto *fns = context()->versionFunctions<QOpenGLFunctions_3_2_Core>();

  fns->glViewport(0, 0, static_cast<GLsizei>(w * pixelRatio),
                  static_cast<GLsizei>(h * pixelRatio));
}

void TestWindow::paintGL() { toneMapper_->map(); }

void TestWindow::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_R:
      histogramGenerator_->clear();
      break;
    case Qt::Key_Escape:
      close();
      break;
    default:
      return;
  }
}

void TestWindow::syncHistogram() {
  histogramGenerator_->withHistogram([this](const HistogramBuffer &histogram) {
    toneMapper_->syncBuffer(histogram);
  });
  update();
}

}  // namespace chaoskit::ui
