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

  auto *colorMapRegistry = new ColorMapRegistry(this);

  auto *model = new DocumentModel(this);
  model->setColorMapRegistry(colorMapRegistry);
  QModelIndex blendIndex = model->addBlend(FormulaType::DeJong);
  model->setData(blendIndex, "Distance", DocumentModel::ColoringMethodTypeRole);
  model->setData(blendIndex, 0.4, DocumentModel::ColoringMethodParamRole);
  QModelIndex formulaIndex = model->formulaAt(0, blendIndex);
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
  histogramGenerator_->setSystem(*model->document()->system);
  histogramGenerator_->setColorMap(
      colorMapRegistry->get(std::string("Rainbow")));
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
      histogramGenerator_->reset();
      break;
    case Qt::Key_Escape:
      close();
      break;
    default:
      return;
  }
}

void TestWindow::syncHistogram() {
  histogramGenerator_->synchronizeResult(this);
  update();
}

void TestWindow::updateHistogramBuffer(const core::HistogramBuffer &buffer) {
  toneMapper_->syncBuffer(buffer);
}

}  // namespace chaoskit::ui
