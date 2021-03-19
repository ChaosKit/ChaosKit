#ifndef CHAOSKIT_UI_STORAGE_CONVERSIONS_H
#define CHAOSKIT_UI_STORAGE_CONVERSIONS_H

#include <chaoskit.pb.h>
#include "core/CameraSystem.h"

namespace chaoskit::ui {

core::CameraSystem toCameraSystem(const System& systemProto);

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_STORAGE_CONVERSIONS_H
