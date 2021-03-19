#ifndef CHAOSKIT_CORE_TRANSFORMSYSTEM_H
#define CHAOSKIT_CORE_TRANSFORMSYSTEM_H

#include "TransformParams.h"

// This is because of circular dependencies in AST. It's important to include
// all transform types here, otherwise core won't have access to them.
#include "ast/transforms.h"

namespace chaoskit::core {

struct TransformSystem {
  ast::Transform transform;
  TransformParams params;

  bool operator==(const TransformSystem& rhs) const {
    return transform == rhs.transform && params == rhs.params;
  }
  bool operator!=(const TransformSystem& rhs) const { return !(rhs == *this); }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TRANSFORMSYSTEM_H
