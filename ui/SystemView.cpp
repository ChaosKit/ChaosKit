#include "SystemView.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QQuickItem>
#include <QQuickWindow>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include "GLToneMapper.h"
#include "core/HistogramBuffer.h"
#include "ui/storage/conversions.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit::ui {

Q_LOGGING_CATEGORY(systemViewLog, "SystemView");

namespace {

class HistogramRenderer : public QQuickFramebufferObject::Renderer {
 public:
  explicit HistogramRenderer(const SystemView *view) : driver_(view->driver()) {
    if (auto *win = view->window()) {
      win->beginExternalCommands();
      toneMapper_.initializeGL();
      win->endExternalCommands();
      initialized_ = true;
    }
  }

 protected:
  void synchronize(QQuickFramebufferObject *object) override {
    auto *view = qobject_cast<SystemView *>(object);
    if (view == nullptr) {
      return;
    }
    window_ = view->window();
    if (window_ == nullptr) {
      return;
    }
    if (!initialized_) {
      window_->beginExternalCommands();
      toneMapper_.initializeGL();
      window_->endExternalCommands();
      initialized_ = true;
    }

    if (const auto *model = view->model()) {
      toneMapper_.setGamma(model->gamma());
      toneMapper_.setExposure(model->exposure());
      toneMapper_.setVibrancy(model->vibrancy());
    }

    if (driver_->syncSnapshotIfChanged(lastSnapshotVersion_, histogramCopy_)) {
      window_->beginExternalCommands();
      if (histogramCopy_.width() > 0 && histogramCopy_.height() > 0) {
        toneMapper_.syncBuffer(histogramCopy_);
      } else {
        static const HistogramBuffer emptyBuffer(1, 1);
        toneMapper_.syncBuffer(emptyBuffer);
      }
      window_->endExternalCommands();
      hasTexture_ = true;
    }
  }

  void render() override {
    if (window_ == nullptr || !initialized_ || !hasTexture_) {
      return;
    }
    window_->beginExternalCommands();
    toneMapper_.map();
    window_->endExternalCommands();
  }

 private:
  GLToneMapper toneMapper_;
  CpuAccumulatorDriver *driver_;
  QQuickWindow *window_ = nullptr;
  HistogramBuffer histogramCopy_;
  uint64_t lastSnapshotVersion_ = 0;
  bool initialized_ = false;
  bool hasTexture_ = false;
};

}  // namespace

SystemView::SystemView(QQuickItem *parent) : QQuickFramebufferObject(parent) {
  driver_ = new CpuAccumulatorDriver(this);
  connect(driver_, &CpuAccumulatorDriver::batchCompleted, this,
          &QQuickItem::update);
  connect(driver_, &CpuAccumulatorDriver::runningChanged, this,
          &SystemView::runningChanged);
  connect(driver_, &CpuAccumulatorDriver::errorOccurred, this,
          [this](const QString &message) {
            qCWarning(systemViewLog, "%s", qPrintable(message));
            emit warningEmitted(message);
          });

  connect(this, &QQuickItem::widthChanged, this, &SystemView::updateBufferSize);
  connect(this, &QQuickItem::heightChanged, this,
          &SystemView::updateBufferSize);
}

QQuickFramebufferObject::Renderer *SystemView::createRenderer() const {
  return new HistogramRenderer(this);
}

void SystemView::clear() {
  driver_->clear();
  update();
}

void SystemView::setRunning(bool running) { driver_->setRunning(running); }

void SystemView::setModel(ProjectModel *projectModel) {
  if (projectModel == model_) {
    return;
  }
  if (model_ != nullptr) {
    model_->disconnect(this);
    model_->system()->disconnect(this);
    model_->colorMap()->disconnect(this);
  }

  model_ = projectModel;
  if (model_ == nullptr) {
    driver_->clearSystem();
    emit modelChanged();
    update();
    return;
  }

  connect(model_, &ProjectModel::gammaChanged, this, &QQuickItem::update);
  connect(model_, &ProjectModel::exposureChanged, this, &QQuickItem::update);
  connect(model_, &ProjectModel::vibrancyChanged, this, &QQuickItem::update);
  connect(model_->system(), &SystemModel::visuallyChanged, this,
          &SystemView::updateSystem);
  connect(model_->colorMap(), &ColorMapModel::nameChanged, this,
          &SystemView::updateColorMap);

  updateSystem();
  updateColorMap();
  updateBufferSize();
  emit modelChanged();
}

void SystemView::updateSystem() {
  if (model_ == nullptr) {
    return;
  }
  const System &systemProto = model_->proto()->system();

  std::vector<std::string> warnings;
  driver_->setSystem(toCameraSystem(systemProto, &warnings));
  for (const auto &warning : warnings) {
    qCWarning(systemViewLog, "%s", warning.c_str());
    emit warningEmitted(QString::fromStdString(warning));
  }
  update();
}

void SystemView::updateColorMap() {
  if (model_ == nullptr) {
    return;
  }
  driver_->setColorMap(model_->colorMap()->coreColorMap());
  update();
}

void SystemView::updateBufferSize() {
  const auto w = static_cast<uint32_t>((std::max)(0.0, width()));
  const auto h = static_cast<uint32_t>((std::max)(0.0, height()));
  driver_->setSize(w, h);
  update();
}

}  // namespace chaoskit::ui
