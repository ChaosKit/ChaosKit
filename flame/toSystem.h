#ifndef CHAOSKIT_FLAME_TOSYSTEM_H
#define CHAOSKIT_FLAME_TOSYSTEM_H

#include "System.h"
#include "core/CameraSystem.h"
#include "toParams.h"
#include "toTransform.h"

namespace chaoskit::flame {

core::CameraSystem toSystem(const System& system) {
  core::CameraSystem result{{toTransform(system), toParams(system)}};

  auto cameraTransform = toCameraTransform(system);
  if (cameraTransform) {
    result.camera = {*std::move(cameraTransform), *toCameraParams(system)};
  }

  return result;
}

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_TOSYSTEM_H
