#ifndef CHAOSKIT_CORE_PARTICLE_H
#define CHAOSKIT_CORE_PARTICLE_H

#include "Point.h"

namespace chaoskit::core {

struct Particle {
  Particle() : point() {}
  Particle(const Particle&) = default;
  explicit Particle(Point point, float color = .5f)
      : point(point), color(color) {}
  Particle(float x, float y, float color = .5f) : point{x, y}, color(color) {}

  Point point;
  float color = .5f;

  [[nodiscard]] float x() const { return point.x(); }
  [[nodiscard]] float y() const { return point.y(); }

  Particle& operator=(const Particle&) = default;

  bool operator==(const Particle& other) const {
    return point == other.point && color == other.color;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_PARTICLE_H
