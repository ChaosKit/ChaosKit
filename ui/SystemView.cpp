#include "SystemView.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickItem>
#include <QQuickWindow>
#include "GLToneMapper.h"
#include "core/Renderer.h"
#include "ui/storage/conversions.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit::ui {

Q_LOGGING_CATEGORY(systemViewLog, "SystemView");

namespace {

class HistogramRenderer : public QQuickFramebufferObject::Renderer,
                          public core::Renderer {
 public:
  explicit HistogramRenderer(const SystemView *view)
      : toneMapper_(), systemView_(view) {
    toneMapper_.initializeGL();
  };

  void updateHistogramBuffer(const HistogramBuffer &buffer) override {
    toneMapper_.syncBuffer(buffer);
  }

 protected:
  void synchronize(QQuickFramebufferObject *object) override {
    systemView_->synchronizeResult(this);

    systemView_ = qobject_cast<const SystemView *>(object);
    const auto *model = systemView_->model();
    toneMapper_.setGamma(model->gamma());
    toneMapper_.setExposure(model->exposure());
    toneMapper_.setVibrancy(model->vibrancy());
  }

  void render() override {
    toneMapper_.map();
    update();

    systemView_->window()->resetOpenGLState();
  }

 private:
  GLToneMapper toneMapper_;
  const SystemView *systemView_;
};

}  // namespace

SystemView::SystemView(QQuickItem *parent) : QQuickFramebufferObject(parent) {
  generator_ = new HistogramGenerator(this);

  connect(this, &QQuickItem::widthChanged, this, &SystemView::updateBufferSize);
  connect(this, &QQuickItem::heightChanged, this,
          &SystemView::updateBufferSize);
}

QQuickFramebufferObject::Renderer *SystemView::createRenderer() const {
  return new HistogramRenderer(this);
}

void SystemView::clear() { generator_->reset(); }

void SystemView::setRunning(bool running) {
  if (generator_->isEnabled() == running) {
    return;
  }

  generator_->setEnabled(running);
  emit runningChanged();
}

void SystemView::setModel(ProjectModel *projectModel) {
  if (projectModel == model_) {
    return;
  }
  if (model_ != nullptr) {
    model_->disconnect(this);
  }

  model_ = projectModel;
  connect(projectModel, &ProjectModel::gammaChanged, this, &QQuickItem::update);
  connect(projectModel, &ProjectModel::exposureChanged, this,
          &QQuickItem::update);
  connect(projectModel, &ProjectModel::vibrancyChanged, this,
          &QQuickItem::update);
  connect(projectModel->system(), &SystemModel::visuallyChanged, this,
          &SystemView::updateSystem);
  connect(projectModel->colorMap(), &ColorMapModel::nameChanged, this,
          &SystemView::updateColorMap);

  updateSystem();
  updateColorMap();
  emit modelChanged();
}

void SystemView::updateSystem() {
  const System &systemProto = model_->proto()->system();

  if (systemProto.skip() < 0) {
    qCWarning(systemViewLog, "Particle skip is less than 0 (%d)",
              systemProto.skip());
  }
  if (systemProto.ttl() < 1 &&
      systemProto.ttl() != core::SystemParticle::IMMORTAL) {
    qCWarning(systemViewLog, "Particle lifetime is 0 or less (%d)",
              systemProto.ttl());
  }

  generator_->setSystem(toCameraSystem(systemProto));
  generator_->setLifetimeRange(systemProto.skip(), systemProto.ttl());
  update();
}

void SystemView::updateColorMap() {
  generator_->setColorMap(model_->colorMap()->coreColorMap());
  update();
}

void SystemView::updateBufferSize() {
  generator_->setSize(static_cast<quint32>(width()),
                      static_cast<quint32>(height()));
}

}  // namespace chaoskit::ui
