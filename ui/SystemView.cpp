#include "SystemView.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickItem>
#include <QQuickWindow>
#include "GLToneMapper.h"
#include "core/Renderer.h"
#include "flame/toSystem.h"

using chaoskit::core::HistogramBuffer;

namespace chaoskit::ui {

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

void SystemView::setModel(DocumentModel *documentModel) {
  if (documentModel == model_) {
    return;
  }
  if (model_ != nullptr) {
    model_->disconnect(this);
  }

  model_ = documentModel;
  updateSystem();
  emit modelChanged();

  connect(documentModel, &DocumentModel::structureChanged, this,
          &SystemView::updateSystem);
}

void SystemView::updateSystem() {
  generator_->setSystem(flame::toCameraSystem(*model_->system()));
  update();
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

void SystemView::setColorMapRegistry(ColorMapRegistry *colorMapRegistry) {
  if (colorMapRegistry_ == colorMapRegistry) {
    return;
  }

  colorMapRegistry_ = colorMapRegistry;
  updateColorMap();
  update();
  emit colorMapRegistryChanged();
}

void SystemView::setColorMap(const QString &name) {
  if (colorMap_ == name) {
    return;
  }

  colorMap_ = name;
  updateColorMap();
  update();
  emit colorMapChanged();
}

void SystemView::updateColorMap() {
  if (!colorMapRegistry_ || colorMap_.isEmpty()) {
    return;
  }

  generator_->setColorMap(colorMapRegistry_->get(colorMap_));
}

void SystemView::updateBufferSize() {
  generator_->setSize(static_cast<quint32>(width()),
                      static_cast<quint32>(height()));
}

}  // namespace chaoskit::ui
