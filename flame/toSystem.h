#ifndef CHAOSKIT_FLAME_TOSYSTEM_H
#define CHAOSKIT_FLAME_TOSYSTEM_H

#include "System.h"
#include "core/CameraSystem.h"
#include "toParams.h"
#include "toTransform.h"

namespace chaoskit::flame {

core::TransformSystem toTransformSystem(const System& system);
core::CameraSystem toCameraSystem(const System& system);

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_TOSYSTEM_H
