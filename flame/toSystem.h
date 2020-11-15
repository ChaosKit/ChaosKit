#ifndef CHAOSKIT_FLAME_TOSYSTEM_H
#define CHAOSKIT_FLAME_TOSYSTEM_H

#include "System.h"
#include "core/CameraSystem.h"
#include "toParams.h"
#include "toTransform.h"

namespace chaoskit::flame {

core::TransformSystem toTransformSystem(const System& system) {
  return {toTransform(system), toParams(system)};
}

core::CameraSystem toCameraSystem(const System& system) {
  core::CameraSystem result{toTransformSystem(system)};

  auto cameraTransform = toCameraTransform(system);
  if (cameraTransform) {
    result.camera = {*std::move(cameraTransform), *toCameraParams(system)};
  }

  return result;
}

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_TOSYSTEM_H
