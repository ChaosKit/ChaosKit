#include "HistogramGenerator.h"
#include <QDebug>
#include "core/Renderer.h"
#include "core/ThreadLocalRng.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::core::ThreadLocalRng;

namespace chaoskit::ui {

HistogramGenerator::HistogramGenerator(QObject *parent) : QObject(parent) {
  thread_ = new QThread();
  thread_->setObjectName("HistogramGenerator");

  blenderTask_ = new BlenderTask(std::make_shared<ThreadLocalRng>());
  blenderTask_->moveToThread(thread_);
  gathererTask_ = new GathererTask();
  gathererTask_->moveToThread(thread_);

  connect(thread_, &QThread::finished, blenderTask_, &QObject::deleteLater);
  connect(thread_, &QThread::finished, gathererTask_, &QObject::deleteLater);
  connect(blenderTask_, &BlenderTask::started, this,
          &HistogramGenerator::started);
  connect(blenderTask_, &BlenderTask::stopped, this,
          &HistogramGenerator::stopped);

  connect(blenderTask_, &BlenderTask::stepCompleted, gathererTask_,
          &GathererTask::addPoint);

  thread_->start();
}

HistogramGenerator::~HistogramGenerator() {
  stop();
  thread_->quit();
  thread_->wait();
}

void HistogramGenerator::synchronizeResult(core::Renderer *renderer) {
  // No MetaObject because the mutex handles synchronization.
  gathererTask_->synchronizeResult(renderer);
}

void HistogramGenerator::setEnabled(bool enabled) {
  if (enabled == isEnabled()) return;

  if (enabled) {
    QMetaObject::invokeMethod(blenderTask_, &BlenderTask::start);
  } else {
    QMetaObject::invokeMethod(blenderTask_, &BlenderTask::stop);
  }

  core::Generator::setEnabled(enabled);
}

void HistogramGenerator::setSystem(const core::CameraSystem &system) {
  QMetaObject::invokeMethod(
      // We capture by value here because if we pass a temporary as the system,
      // the memory might get inaccessible. Capturing by value ensures that a
      // copy gets made.
      blenderTask_, [this, system] { blenderTask_->setSystem(&system); });
  gathererTask_->clear();
}

void HistogramGenerator::setLifetimeRange(int32_t min, int32_t max) {
  QMetaObject::invokeMethod(blenderTask_, [this, min, max] {
    blenderTask_->setLifetimeRange(min, max);
  });
  gathererTask_->clear();
}

void HistogramGenerator::setColorMap(const chaoskit::core::ColorMap *colorMap) {
  QMetaObject::invokeMethod(gathererTask_, [colorMap, this] {
    gathererTask_->setColorMap(colorMap);
  });
}
void HistogramGenerator::setSize(quint32 width, quint32 height) {
  QMetaObject::invokeMethod(
      blenderTask_, [=] { gathererTask_->setSize(QSize(width, height)); });
}
void HistogramGenerator::start() { setEnabled(true); }
void HistogramGenerator::stop() { setEnabled(false); }
void HistogramGenerator::reset() {
  // No MetaObject because the mutex handles synchronization.
  gathererTask_->clear();
}

}  // namespace chaoskit::ui
