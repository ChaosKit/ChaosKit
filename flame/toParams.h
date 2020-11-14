#ifndef CHAOSKIT_FLAME_TOPARAMS_H
#define CHAOSKIT_FLAME_TOPARAMS_H

#include "System.h"
#include "core/TransformParams.h"

namespace chaoskit::flame {

core::TransformParams toParams(const System& system);

}

#endif  // CHAOSKIT_FLAME_TOPARAMS_H
