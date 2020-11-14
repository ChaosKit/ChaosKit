#ifndef CHAOSKIT_FLAME_TOSYSTEM_H
#define CHAOSKIT_FLAME_TOSYSTEM_H

#include "System.h"
#include "core/TransformSystem.h"
#include "toAst.h"
#include "toParams.h"

namespace chaoskit::flame {

core::TransformSystem toSystem(const System& system) {
  return {toAst(system), toParams(system)};
}

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_TOSYSTEM_H
