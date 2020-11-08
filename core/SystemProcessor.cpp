#include "SystemProcessor.h"

namespace chaoskit::core {

SystemParticle SystemProcessor::createParticle() const {
  SystemParticle particle;
  reviveParticle(particle.particle);
  particle.particle.ttl = (lifetime_ == Particle::IMMORTAL)
                              ? Particle::IMMORTAL
                              : rng_->randomInt(1, lifetime_);
  particle.particle.skip = skip_;
  return particle;
}

SystemParticle SystemProcessor::process() const {
  return process(createParticle());
}

SystemParticle SystemProcessor::process(SystemParticle input) const {
  SystemParticle output = input;

  if (output.particle.ttl == 0) {
    reviveParticle(output.particle);
    output.particle.ttl = lifetime_;
    output.particle.skip = skip_;
  }

  for (; output.particle.skip >= 0; --output.particle.skip) {
    output.particle = interpreter_.interpret(output.particle, system_.transform,
                                             system_.params);

    if (output.particle.ttl != Particle::IMMORTAL) {
      --output.particle.ttl;
    }

    if (output.particle.ttl == 0) {
      break;
    }
  }
  // Fix the skip state after the for loop.
  output.particle.skip = 0;

  return output;
}

void SystemProcessor::reviveParticle(Particle &particle) const {
  particle.point = {rng_->randomFloat(bounds_.left, bounds_.right),
                    rng_->randomFloat(bounds_.top, bounds_.bottom)};
  particle.color = rng_->randomFloat(0.f, 1.f);
}

}  // namespace chaoskit::core
