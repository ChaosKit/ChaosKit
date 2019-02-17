#include "BlenderTask.h"
#include <QTimer>
#include "models/toSource.h"

using chaoskit::core::Particle;
using chaoskit::core::Point;
using chaoskit::core::SimpleInterpreter;
using chaoskit::ui::toSource;

namespace chaoskit {
namespace ui {

namespace {

float distance(const Point &a, const Point &b) {
  float dx = a.x() - b.x();
  float dy = a.y() - b.y();
  return sqrtf(dx * dx + dy * dy);
}

}  // namespace

void BlenderTask::setSystem(const models::System *system) {
  interpreter_ = std::make_unique<SimpleInterpreter>(toSource(system),
                                                     system->params(), rng_);
  resetParticle();
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

  auto[next_state, output] = (*interpreter_)(particle_.point);
  // TODO: calculate color in AST
  float unused;
  particle_.color = modf(distance(particle_.point, next_state) * .2f, &unused);

  particle_.point = next_state;
  emit stepCompleted(output, particle_.color);

  if (particle_.ttl == 0) {
    randomizeParticle();
    particle_.ttl = ttl_;
  } else if (particle_.ttl != Particle::IMMORTAL) {
    --particle_.ttl;
  }

  QTimer::singleShot(0, this, &BlenderTask::calculate);
}

void BlenderTask::setTtl(int32_t ttl) {
  ttl_ = ttl;
  resetParticle();
}

void BlenderTask::randomizeParticle() {
  particle_.point =
      Point(rng_->randomFloat(-1.f, 1.f), rng_->randomFloat(-1.f, 1.f));
  particle_.color = rng_->randomFloat(0.f, 1.f);
}

void BlenderTask::resetParticle() {
  randomizeParticle();
  particle_.ttl = (ttl_ == Particle::IMMORTAL) ? Particle::IMMORTAL
                                               : rng_->randomInt(1, ttl_);
}

}  // namespace ui
}  // namespace chaoskit
