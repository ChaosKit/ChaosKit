#ifndef CHAOSKIT_AST_TRANSFORMWITHWEIGHT_H
#define CHAOSKIT_AST_TRANSFORMWITHWEIGHT_H

#include <utility>
#include "Transform.h"

namespace chaoskit::ast {

struct TransformWithWeight {
  // Allow implicitly lifting a Transform to TransformWithWeight.
  // NOLINTNEXTLINE(google-explicit-constructor)
  TransformWithWeight(Transform transform) : transform(std::move(transform)) {}

  Transform transform;
  double weight = 1.0;

  bool operator==(const TransformWithWeight& other) const {
    return transform == other.transform && weight == other.weight;
  }
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_TRANSFORMWITHWEIGHT_H