#include "MainWindow.h"
#include "RenderingForm.h"
#include "SystemView.h"
#include "ui_MainWindow.h"

namespace chaoskit {
namespace ui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  renderingForm_ = new RenderingForm(this);
  addDockWidget(Qt::RightDockWidgetArea, renderingForm_);

  auto *systemView = findSystemView();
  connect(systemView, &SystemView::started, this,
          &MainWindow::forwardRunStatus);
  connect(systemView, &SystemView::stopped, this,
          &MainWindow::forwardRunStatus);
  connect(renderingForm_, &RenderingForm::startStopClicked, this,
          &MainWindow::toggleGeneration);
  connect(renderingForm_, &RenderingForm::clearClicked, systemView,
          &SystemView::clear);
  connect(renderingForm_, &RenderingForm::gammaChanged, systemView,
          &SystemView::setGamma);
  connect(renderingForm_, &RenderingForm::exposureChanged, systemView,
          &SystemView::setExposure);
  connect(renderingForm_, &RenderingForm::vibrancyChanged, systemView,
          &SystemView::setVibrancy);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::forwardRunStatus() {
  auto *systemView = findSystemView();
  renderingForm_->updateStartStopLabel(systemView->running());
}

void MainWindow::toggleGeneration() {
  auto *systemView = findSystemView();
  if (systemView->running()) {
    systemView->stop();
  } else {
    systemView->start();
  }
}

SystemView *MainWindow::findSystemView() const {
  return ui->editor->rootObject()->findChild<SystemView *>("systemview");
}

}  // namespace ui
}  // namespace chaoskit
