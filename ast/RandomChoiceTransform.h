#ifndef CHAOSKIT_AST_RANDOMCHOICETRANSFORM_H
#define CHAOSKIT_AST_RANDOMCHOICETRANSFORM_H

#include "TransformWithWeight.h"

namespace chaoskit::ast {

class RandomChoiceTransform {
 public:
  RandomChoiceTransform() = default;
  RandomChoiceTransform(std::initializer_list<TransformWithWeight> transforms)
      : transforms_(transforms) {}
  explicit RandomChoiceTransform(std::vector<TransformWithWeight> transforms)
      : transforms_(std::move(transforms)) {}

  [[nodiscard]] const std::vector<TransformWithWeight>& transforms() const {
    return transforms_;
  }

  bool operator==(const RandomChoiceTransform& other) const {
    return transforms_ == other.transforms_;
  }

 private:
  std::vector<TransformWithWeight> transforms_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_RANDOMCHOICETRANSFORM_H
