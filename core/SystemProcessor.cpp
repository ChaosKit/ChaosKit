#include "SystemProcessor.h"
#include <algorithm>
#include "SystemParticle.h"

namespace chaoskit::core {

SystemParticle SystemProcessor::createParticle() const {
  SystemParticle particle;
  reviveParticle(particle);
  particle.lifetime =
      (max_lifetime_ == SystemParticle::INFINITE_LIFETIME)
          ? SystemParticle::INFINITE_LIFETIME
          : rng_->randomInt((std::min)(min_lifetime_ + 1, max_lifetime_),
                            max_lifetime_);
  particle.warmup = min_lifetime_;
  return particle;
}

SystemParticle SystemProcessor::process() const {
  return process(createParticle());
}

SystemParticle SystemProcessor::process(SystemParticle input) const {
  SystemParticle output = input;

  if (output.lifetime == 0) {
    reviveParticle(output);
    output.lifetime = max_lifetime_;
    output.warmup = min_lifetime_;
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
