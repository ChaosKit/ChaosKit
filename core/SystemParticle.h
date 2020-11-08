#ifndef CHAOSKIT_CORE_SYSTEMPARTICLE_H
#define CHAOSKIT_CORE_SYSTEMPARTICLE_H

#include "Particle.h"

namespace chaoskit::core {

// TODO: move ttl and skip from Particle to this class
struct SystemParticle {
  Particle particle;

  bool operator==(const SystemParticle& other) const {
    return particle == other.particle;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEMPARTICLE_H
