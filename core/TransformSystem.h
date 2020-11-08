#ifndef CHAOSKIT_CORE_TRANSFORMSYSTEM_H
#define CHAOSKIT_CORE_TRANSFORMSYSTEM_H

#include "TransformParams.h"
#include "ast/Transform.h"

namespace chaoskit::core {

struct TransformSystem {
  ast::Transform transform;
  TransformParams params;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TRANSFORMSYSTEM_H
