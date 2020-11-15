#ifndef CHAOSKIT_CORE_DEBUG_H
#define CHAOSKIT_CORE_DEBUG_H

#include <ostream>
#include "CameraSystem.h"
#include "Particle.h"
#include "Point.h"
#include "TransformParams.h"
#include "TransformSystem.h"

namespace chaoskit::core {

// Basic types

std::ostream& operator<<(std::ostream& stream, const Point& point);
std::ostream& operator<<(std::ostream& stream, const Particle& particle);

// Transform-related stuff

std::ostream& operator<<(std::ostream& stream, const TransformParams& params);
std::ostream& operator<<(std::ostream& stream, const TransformSystem& system);
std::ostream& operator<<(std::ostream& stream, const CameraSystem& system);

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_DEBUG_H
