#include "debug.h"

namespace chaoskit::core {

std::ostream& operator<<(std::ostream& stream, const Point& point) {
  return stream << "(" << point.x() << ", " << point.y() << ")";
}

std::ostream& operator<<(std::ostream& stream, const Particle& particle) {
  return stream << "[" << particle.point << ", " << particle.color << "]";
}

}  // namespace chaoskit::core
