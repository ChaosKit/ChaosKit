#include "RenderingForm.h"
#include "ui_RenderingForm.h"

namespace chaoskit {
namespace ui {

RenderingForm::RenderingForm(QWidget* parent)
    : QDockWidget(parent), ui(new Ui::RenderingForm) {
  ui->setupUi(this);

  connect(ui->startStopButton, &QPushButton::clicked, this,
          &RenderingForm::startStopClicked);
  connect(ui->clearButton, &QPushButton::clicked, this,
          &RenderingForm::clearClicked);
  connect(ui->resetButton, &QPushButton::clicked, this,
          &RenderingForm::resetParameters);
  connect(ui->gammaSlider, &QSlider::valueChanged, this,
          &RenderingForm::forwardGamma);
  connect(ui->exposureSlider, &QSlider::valueChanged, this,
          &RenderingForm::forwardExposure);
  connect(ui->vibrancySlider, &QSlider::valueChanged, this,
          &RenderingForm::forwardVibrancy);
}

RenderingForm::~RenderingForm() { delete ui; }

void RenderingForm::updateStartStopLabel(bool running) {
  ui->startStopButton->setText(running ? QStringLiteral("Stop")
                                       : QStringLiteral("Start"));
}

void RenderingForm::forwardGamma(int gamma) { emit gammaChanged(gamma * .1f); }

void RenderingForm::forwardExposure(int exposure) {
  emit exposureChanged(exposure * .1f);
}

void RenderingForm::forwardVibrancy(int vibrancy) {
  emit vibrancyChanged(vibrancy * .01f);
}

void RenderingForm::resetParameters() {
  ui->gammaSlider->setValue(22);
  ui->exposureSlider->setValue(0);
  ui->vibrancySlider->setValue(0);
  update();
}

}  // namespace ui
}  // namespace chaoskit
