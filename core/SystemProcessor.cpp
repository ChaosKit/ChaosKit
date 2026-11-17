#include "SystemProcessor.h"
#include <cmath>
#include "SystemParticle.h"

namespace chaoskit::core {

SystemParticle SystemProcessor::createParticle() const {
  SystemParticle particle;
  reviveParticle(particle);
  particle.lifetime = lifetime_.generateInitialLifetime(*rng_);
  particle.warmup = lifetime_.min();
  return particle;
}

SystemParticle SystemProcessor::process() const {
  return process(createParticle());
}

bool SystemProcessor::step(SystemParticle &particle) const {
  if (particle.lifetime == 0) {
    reviveParticle(particle);
    particle.lifetime = lifetime_.revivedLifetime();
    particle.warmup = lifetime_.min();
  }

  if (!stepParticle(particle)) {
    return false;
  }

  if (particle.warmup > 0) {
    --particle.warmup;
    return false;
  }

  return true;
}

SystemParticle SystemProcessor::process(SystemParticle input) const {
  SystemParticle output = input;

  if (output.lifetime == 0) {
    reviveParticle(output);
    output.lifetime = lifetime_.revivedLifetime();
    output.warmup = lifetime_.min();
  }

  for (; output.warmup > 0; --output.warmup) {
    if (!stepParticle(output) || output.lifetime == 0) {
      break;
    }
  }

  if (output.lifetime != 0) {
    stepParticle(output);
  }

  output.warmup = 0;

  return output;
}

bool SystemProcessor::stepParticle(SystemParticle &particle) const {
  particle.applyParticle(interpreter_.interpret(
      particle.asParticle(), system_.transform, system_.params));

  if (!std::isfinite(particle.x()) || !std::isfinite(particle.y()) ||
      !std::isfinite(particle.color)) {
    particle.lifetime = 0;
    particle.warmup = 0;
    return false;
  }

  if (particle.lifetime != SystemParticle::INFINITE_LIFETIME) {
    --particle.lifetime;
  }
  return true;
}

void SystemProcessor::reviveParticle(Particle &particle) const {
  particle.point = {rng_->randomFloat(bounds_.left, bounds_.right),
                    rng_->randomFloat(bounds_.top, bounds_.bottom)};
  particle.color = rng_->randomFloat(0.f, 1.f);
}

}  // namespace chaoskit::core
