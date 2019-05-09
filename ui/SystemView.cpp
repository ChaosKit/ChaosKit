#include "SystemView.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickItem>
#include <QQuickWindow>
#include "GLToneMapper.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit::ui {

namespace {

class HistogramRenderer : public QQuickFramebufferObject::Renderer {
 public:
  explicit HistogramRenderer(const SystemView *view)
      : toneMapper_(), systemView_(view) {
    toneMapper_.initializeGL();
  };

 protected:
  void synchronize(QQuickFramebufferObject *object) override {
    systemView_->withHistogram([this](const HistogramBuffer &histogram) {
      toneMapper_.syncBuffer(histogram);
    });

    systemView_ = qobject_cast<const SystemView *>(object);
    toneMapper_.setGamma(systemView_->gamma());
    toneMapper_.setExposure(systemView_->exposure());
    toneMapper_.setVibrancy(systemView_->vibrancy());
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
  connect(generator_, &HistogramGenerator::started, this,
          &SystemView::runningChanged);
  connect(generator_, &HistogramGenerator::stopped, this,
          &SystemView::runningChanged);
}

void SystemView::withHistogram(
    const std::function<void(const HistogramBuffer &)> &action) const {
  generator_->withHistogram(action);
}

QQuickFramebufferObject::Renderer *SystemView::createRenderer() const {
  return new HistogramRenderer(this);
}

void SystemView::start() { generator_->start(); }

void SystemView::stop() { generator_->stop(); }

void SystemView::clear() { generator_->clear(); }

void SystemView::setRunning(bool running) {
  if (generator_->running() == running) {
    return;
  }

  if (running) {
    start();
  } else {
    stop();
  }
}

void SystemView::setSystem(System *system) {
  if (system == system_) {
    return;
  }

  if (system_ != nullptr) {
    system_->disconnect(this);
  }

  system_ = system;
  updateSystem();
  emit systemChanged();

  connect(system, &System::sourceChanged, this, &SystemView::updateSystem);
  connect(system, &System::paramsChanged, this, &SystemView::updateSystem);
  connect(system, &System::finalBlendSourceChanged, this,
          &SystemView::updateSystem);
  connect(system, &System::finalBlendParamsChanged, this,
          &SystemView::updateSystem);
}

void SystemView::updateSystem() {
  generator_->setSystem(system_);
  generator_->clear();
  update();
}

void SystemView::setTtl(int ttl) {
  if (ttl == ttl_) {
    return;
  }

  ttl_ = ttl;
  generator_->setTtl(ttl);
  generator_->clear();
  update();
  emit ttlChanged();
}

void SystemView::setGamma(float gamma) {
  if (qFuzzyCompare(gamma_, gamma)) {
    return;
  }

  gamma_ = gamma;
  update();
  emit gammaChanged();
}

void SystemView::setExposure(float exposure) {
  if (qFuzzyCompare(exposure_, exposure)) {
    return;
  }

  exposure_ = exposure;
  update();
  emit exposureChanged();
}

void SystemView::setVibrancy(float vibrancy) {
  if (qFuzzyCompare(vibrancy_, vibrancy)) {
    return;
  }

  vibrancy_ = vibrancy;
  update();
  emit vibrancyChanged();
}

void SystemView::updateBufferSize() {
  generator_->setSize(static_cast<quint32>(width()),
                      static_cast<quint32>(height()));
}

}  // namespace chaoskit::ui
