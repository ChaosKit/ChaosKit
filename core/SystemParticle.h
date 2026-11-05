#ifndef CHAOSKIT_CORE_SYSTEMPARTICLE_H
#define CHAOSKIT_CORE_SYSTEMPARTICLE_H

#include <cstdint>
#include "Particle.h"
#include "Point.h"

namespace chaoskit::core {

struct SystemParticle : public Particle {
  static constexpr int32_t INFINITE_LIFETIME = -1;

  SystemParticle() : Particle() {}
  SystemParticle(const SystemParticle&) = default;
  explicit SystemParticle(Point point) : Particle(point) {}
  SystemParticle(float x, float y) : Particle(x, y) {}
  SystemParticle(Point point, float color, int32_t lifetime = 0,
                 int32_t warmup = 0)
      : Particle(point, color), lifetime(lifetime), warmup(warmup) {}
  SystemParticle(float x, float y, float color, int32_t lifetime = 0,
                 int32_t warmup = 0)
      : Particle(x, y, color), lifetime(lifetime), warmup(warmup) {}

  int32_t lifetime = 0;
  int32_t warmup = 0;

  bool operator==(const SystemParticle& other) const {
    return Particle::operator==(other) && lifetime == other.lifetime &&
           warmup == other.warmup;
  }

  Particle& asParticle() { return static_cast<Particle&>(*this); }

  /** Copies the point and color from another particle. */
  SystemParticle& applyParticle(const Particle& particle) {
    Particle::operator=(particle);
    return *this;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEMPARTICLE_H
