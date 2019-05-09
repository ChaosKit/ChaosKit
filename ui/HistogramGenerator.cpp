#include "HistogramGenerator.h"
#include <core/ThreadLocalRng.h>
#include <QDebug>

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::core::ThreadLocalRng;

namespace chaoskit::ui {

HistogramGenerator::HistogramGenerator(QObject *parent) : QObject(parent) {
  thread_ = new QThread();

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

void HistogramGenerator::withHistogram(
    const std::function<void(const HistogramBuffer &)> &action) {
  // No MetaObject because the mutex handles synchronization.
  gathererTask_->withHistogram(action);
}

void HistogramGenerator::setSystem(const System *system) {
  QMetaObject::invokeMethod(blenderTask_,
                            [=] { blenderTask_->setSystem(system); });
}

void HistogramGenerator::setSize(quint32 width, quint32 height) {
  QMetaObject::invokeMethod(
      blenderTask_, [=] { gathererTask_->setSize(QSize(width, height)); });
}

void HistogramGenerator::setTtl(int32_t ttl) {
  QMetaObject::invokeMethod(blenderTask_, [=] { blenderTask_->setTtl(ttl); });
}

void HistogramGenerator::start() {
  QMetaObject::invokeMethod(blenderTask_, &BlenderTask::start);
  running_ = true;
}

void HistogramGenerator::stop() {
  QMetaObject::invokeMethod(blenderTask_, &BlenderTask::stop);
  running_ = false;
}

void HistogramGenerator::clear() {
  // No MetaObject because the mutex handles synchronization.
  gathererTask_->clear();
}

}  // namespace chaoskit::ui
