#include "SystemProcessor.h"
#include "SystemParticle.h"

namespace chaoskit::core {

SystemParticle SystemProcessor::createParticle() const {
  SystemParticle particle;
  reviveParticle(particle);
  particle.ttl = (lifetime_ == SystemParticle::IMMORTAL)
                     ? SystemParticle::IMMORTAL
                     : rng_->randomInt(1, lifetime_);
  particle.skip = skip_;
  return particle;
}

SystemParticle SystemProcessor::process() const {
  return process(createParticle());
}

SystemParticle SystemProcessor::process(SystemParticle input) const {
  SystemParticle output = input;

  if (output.ttl == 0) {
    reviveParticle(output);
    output.ttl = lifetime_;
    output.skip = skip_;
  }

  for (; output.skip >= 0; --output.skip) {
    output.applyParticle(interpreter_.interpret(
        output.asParticle(), system_.transform, system_.params));

    if (output.ttl != SystemParticle::IMMORTAL) {
      --output.ttl;
    }

    if (output.ttl == 0) {
      break;
    }
  }
  // Fix the skip state after the for loop.
  output.skip = 0;

  return output;
}

void SystemProcessor::reviveParticle(Particle &particle) const {
  particle.point = {rng_->randomFloat(bounds_.left, bounds_.right),
                    rng_->randomFloat(bounds_.top, bounds_.bottom)};
  particle.color = rng_->randomFloat(0.f, 1.f);
}

}  // namespace chaoskit::core
