#include "SystemProcessor.h"
#include <algorithm>
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

SystemParticle SystemProcessor::process(SystemParticle input) const {
  SystemParticle output = input;

  if (output.lifetime == 0) {
    reviveParticle(output);
    output.lifetime = lifetime_.revivedLifetime();
    output.warmup = lifetime_.min();
  }

  for (; output.warmup > 0; --output.warmup) {
    stepParticle(output);

    if (output.lifetime == 0) {
      break;
    }
  }

  if (output.lifetime != 0) {
    stepParticle(output);
  }

  output.warmup = 0;

  return output;
}

void SystemProcessor::stepParticle(SystemParticle &particle) const {
  particle.applyParticle(interpreter_.interpret(
      particle.asParticle(), system_.transform, system_.params));

  if (particle.lifetime != SystemParticle::INFINITE_LIFETIME) {
    --particle.lifetime;
  }
}

void SystemProcessor::reviveParticle(Particle &particle) const {
  particle.point = {rng_->randomFloat(bounds_.left, bounds_.right),
                    rng_->randomFloat(bounds_.top, bounds_.bottom)};
  particle.color = rng_->randomFloat(0.f, 1.f);
}

}  // namespace chaoskit::core
