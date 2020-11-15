#ifndef CHAOSKIT_FLAME_TOPARAMS_H
#define CHAOSKIT_FLAME_TOPARAMS_H

#include "System.h"
#include "core/TransformParams.h"
#include "stdx/optional.h"

namespace chaoskit::flame {

core::TransformParams toParams(const System& system);
stdx::optional<core::TransformParams> toCameraParams(const System& system);

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_TOPARAMS_H
