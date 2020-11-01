#ifndef CHAOSKIT_CORE_DEBUG_H
#define CHAOSKIT_CORE_DEBUG_H

#include <ostream>
#include "Particle.h"
#include "Point.h"

namespace chaoskit::core {

std::ostream& operator<<(std::ostream& stream, const Point& point);
std::ostream& operator<<(std::ostream& stream, const Particle& particle);

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_DEBUG_H
