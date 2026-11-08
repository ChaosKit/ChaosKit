#ifndef CHAOSKIT_UI_STORAGE_CONVERSIONS_H
#define CHAOSKIT_UI_STORAGE_CONVERSIONS_H

#include <chaoskit.pb.h>
#include <QString>
#include <vector>
#include "core/CameraSystem.h"
#include "core/TransformSystem.h"

namespace chaoskit::ui {

core::TransformSystem toTransformSystem(const Blend& blendProto,
                                        std::vector<QString>& warnings);
core::TransformSystem toTransformSystem(const Blend& blendProto);

core::CameraSystem toCameraSystem(const System& systemProto,
                                  std::vector<QString>& warnings);
core::CameraSystem toCameraSystem(const System& systemProto);

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_STORAGE_CONVERSIONS_H
