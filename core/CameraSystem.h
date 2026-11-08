#ifndef CHAOSKIT_CORE_CAMERASYSTEM_H
#define CHAOSKIT_CORE_CAMERASYSTEM_H

#include "Lifetime.h"
#include "TransformSystem.h"
#include "stdx/optional.h"

namespace chaoskit::core {

struct CameraSystem {
  TransformSystem system;
  stdx::optional<TransformSystem> camera;
  Lifetime lifetime = Lifetime();

  bool operator==(const CameraSystem& rhs) const {
    return system == rhs.system && camera == rhs.camera &&
           lifetime == rhs.lifetime;
  }
  bool operator!=(const CameraSystem& rhs) const { return !(rhs == *this); }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_CAMERASYSTEM_H
