#include "SystemProcessor.h"
#include <algorithm>
#include "SystemParticle.h"

namespace chaoskit::core {

SystemParticle SystemProcessor::createParticle() const {
  SystemParticle particle;
  reviveParticle(particle);
  particle.ttl =
      (lifetime_ == SystemParticle::IMMORTAL)
          ? SystemParticle::IMMORTAL
          : rng_->randomInt((std::min)(skip_ + 1, lifetime_), lifetime_);
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

  for (; output.skip > 0; --output.skip) {
    stepParticle(output);

    if (output.ttl == 0) {
      break;
    }
  }

  if (output.ttl != 0) {
    stepParticle(output);
  }

  output.skip = 0;

  return output;
}

void SystemProcessor::stepParticle(SystemParticle &particle) const {
  particle.applyParticle(interpreter_.interpret(
      particle.asParticle(), system_.transform, system_.params));

  if (particle.ttl != SystemParticle::IMMORTAL) {
    --particle.ttl;
  }
}

void SystemProcessor::reviveParticle(Particle &particle) const {
  particle.point = {rng_->randomFloat(bounds_.left, bounds_.right),
                    rng_->randomFloat(bounds_.top, bounds_.bottom)};
  particle.color = rng_->randomFloat(0.f, 1.f);
}

}  // namespace chaoskit::core
