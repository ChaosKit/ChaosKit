#include "SystemView.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickWindow>
#include <QQuickItem>
#include "GLToneMapper.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit {
namespace ui {

namespace {

class HistogramRenderer : public QQuickFramebufferObject::Renderer {
 public:
  explicit HistogramRenderer(const SystemView *view)
      : toneMapper_(), systemView_(view) {
    toneMapper_.initializeGL();
  };

 protected:
  void synchronize(QQuickFramebufferObject *object) override {
    systemView_ = dynamic_cast<const SystemView *>(object);
    gamma_ = systemView_->gamma();
    exposure_ = systemView_->exposure();
    vibrancy_ = systemView_->vibrancy();
  }

  void render() override {
    systemView_->withHistogram([this](const HistogramBuffer &histogram) {
      toneMapper_.syncBuffer(histogram);
    });

    toneMapper_.map();
    update();

    systemView_->window()->resetOpenGLState();
  }

 private:
  GLToneMapper toneMapper_;
  const SystemView *systemView_;

  float gamma_;
  float exposure_;
  float vibrancy_;
};

}  // namespace

SystemView::SystemView(QQuickItem *parent) : QQuickFramebufferObject(parent) {
  generator_ = new HistogramGenerator(this);

  connect(this, &QQuickItem::widthChanged, this, &SystemView::updateBufferSize);
  connect(this, &QQuickItem::heightChanged, this, &SystemView::updateBufferSize);
}

void SystemView::withHistogram(
    const std::function<void(const HistogramBuffer &)> &action) const {
  generator_->withHistogram(action);
}

QQuickFramebufferObject::Renderer *SystemView::createRenderer() const {
  return new HistogramRenderer(this);
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
  if (gamma_ == gamma) {
    return;
  }

  gamma_ = gamma;
  update();
  emit gammaChanged();
}

void SystemView::setExposure(float exposure) {
  if (exposure_ == exposure) {
    return;
  }

  exposure_ = exposure;
  update();
  emit exposureChanged();
}

void SystemView::setVibrancy(float vibrancy) {
  if (vibrancy_ == vibrancy) {
    return;
  }

  vibrancy_ = vibrancy;
  update();
  emit vibrancyChanged();
}

void SystemView::componentComplete() {
  QQuickItem::componentComplete();

  System system;
  generator_->setSystem(system.system());
  generator_->setTtl(ttl_);
  generator_->start();
}

void SystemView::updateBufferSize() {
  generator_->setSize(static_cast<quint32>(width()),
                      static_cast<quint32>(height()));
}

}  // namespace ui
}  // namespace chaoskit
