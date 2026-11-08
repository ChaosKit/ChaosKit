#ifndef CHAOSKIT_UI_STORAGE_CONVERSIONS_H
#define CHAOSKIT_UI_STORAGE_CONVERSIONS_H

#include <chaoskit.pb.h>
#include <string>
#include <vector>
#include "core/CameraSystem.h"
#include "core/TransformSystem.h"

namespace chaoskit::ui {

core::TransformSystem toTransformSystem(
    const Blend& blendProto, std::vector<std::string>* warnings = nullptr);
core::CameraSystem toCameraSystem(const System& systemProto,
                                  std::vector<std::string>* warnings = nullptr);

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_STORAGE_CONVERSIONS_H
