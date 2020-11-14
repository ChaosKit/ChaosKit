#include "BlenderTask.h"
#include <QDebug>
#include <QTimer>
#include "core/errors.h"
#include "flame/toSource.h"

using chaoskit::core::MissingParameterError;
using chaoskit::core::Particle;
using chaoskit::core::Point;
using chaoskit::core::SimpleInterpreter;
using chaoskit::core::toSource;
using chaoskit::core::Transform;

namespace chaoskit::ui {

namespace {

float distance(const Point &a, const Point &b) {
  float dx = a.x() - b.x();
  float dy = a.y() - b.y();
  return sqrtf(dx * dx + dy * dy);
}

}  // namespace

void BlenderTask::setSystem(const core::System *system) {
  interpreter_ = std::make_unique<SimpleInterpreter>(*system, rng_);
  particle_ = interpreter_->randomizeParticle();
}

void BlenderTask::start() {
  if (!interpreter_ || running_) {
    return;
  }

  running_ = true;
  emit started();
  QTimer::singleShot(0, this, &BlenderTask::calculate);
}

void BlenderTask::stop() {
  if (!running_) {
    return;
  }

  running_ = false;
  emit stopped();
}

void BlenderTask::calculate() {
  if (!running_) {
    return;
  }

  try {
    auto [next_state, output] = (*interpreter_)(particle_);
    particle_ = next_state;
    emit stepCompleted(output.point, output.color);

    QTimer::singleShot(0, this, &BlenderTask::calculate);
  } catch (MissingParameterError &e) {
    qCritical() << "In BlenderTask::calculate():" << e.what();
    stop();
  }
}

}  // namespace chaoskit::ui
