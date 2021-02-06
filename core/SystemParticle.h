#ifndef CHAOSKIT_CORE_SYSTEMPARTICLE_H
#define CHAOSKIT_CORE_SYSTEMPARTICLE_H

#include <cstdint>
#include "Particle.h"
#include "Point.h"

namespace chaoskit::core {

struct SystemParticle : public Particle {
  static const int32_t IMMORTAL = -1;

  SystemParticle() : Particle() {}
  SystemParticle(const SystemParticle&) = default;
  explicit SystemParticle(Point point) : Particle(point) {}
  SystemParticle(float x, float y) : Particle(x, y) {}
  SystemParticle(Point point, float color, int32_t ttl = 0, int32_t skip = 0)
      : Particle(point, color), ttl(ttl), skip(skip) {}
  SystemParticle(float x, float y, float color, int32_t ttl = 0,
                 int32_t skip = 0)
      : Particle(x, y, color), ttl(ttl), skip(skip) {}

  int32_t ttl = 0;
  int32_t skip = 0;

  bool operator==(const SystemParticle& other) const {
    return Particle::operator==(other) && ttl == other.ttl &&
           skip == other.skip;
  }

  Particle& asParticle() { return static_cast<Particle&>(*this); }

  /** Copies the point and color from another particle. */
  SystemParticle& applyParticle(const Particle& particle) {
    Particle::operator=(particle);
    return *this;
  }

  /** Copies everything from another SystemParticle. */
  SystemParticle& operator=(const SystemParticle& other) {
    Particle::operator=(other);
    ttl = other.ttl;
    skip = other.ttl;
    return *this;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEMPARTICLE_H
