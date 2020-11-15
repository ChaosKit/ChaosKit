#ifndef CHAOSKIT_CORE_CAMERASYSTEM_H
#define CHAOSKIT_CORE_CAMERASYSTEM_H

#include "TransformSystem.h"
#include "stdx/optional.h"

namespace chaoskit::core {

struct CameraSystem {
  TransformSystem system;
  stdx::optional<TransformSystem> camera;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_CAMERASYSTEM_H
