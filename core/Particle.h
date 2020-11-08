#ifndef CHAOSKIT_CORE_PARTICLE_H
#define CHAOSKIT_CORE_PARTICLE_H

#include <cstdint>
#include "Point.h"

namespace chaoskit::core {

struct Particle {
  static const int32_t IMMORTAL = -1;

  Point point;
  float color = .5f;
  int32_t ttl = 0;
  int32_t skip = 0;

  [[nodiscard]] float x() const { return point.x(); }
  [[nodiscard]] float y() const { return point.y(); }

  bool operator==(const Particle& other) const {
    return point == other.point && color == other.color && ttl == other.ttl;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_PARTICLE_H
