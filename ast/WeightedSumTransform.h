#ifndef CHAOSKIT_AST_WEIGHTEDSUMTRANSFORM_H
#define CHAOSKIT_AST_WEIGHTEDSUMTRANSFORM_H

#include "TransformWithWeight.h"

namespace chaoskit::ast {

class WeightedSumTransform {
 public:
  WeightedSumTransform() = default;
  WeightedSumTransform(std::initializer_list<TransformWithWeight> transforms)
      : transforms_(transforms) {}

  [[nodiscard]] const std::vector<TransformWithWeight>& transforms() const {
    return transforms_;
  }

  bool operator==(const WeightedSumTransform& other) const {
    return transforms_ == other.transforms_;
  }

 private:
  std::vector<TransformWithWeight> transforms_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_WEIGHTEDSUMTRANSFORM_H
