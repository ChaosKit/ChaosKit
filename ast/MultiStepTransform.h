#ifndef CHAOSKIT_AST_MULTISTEPTRANSFORM_H
#define CHAOSKIT_AST_MULTISTEPTRANSFORM_H

#include "Transform.h"

namespace chaoskit::ast {

class MultiStepTransform {
 public:
  MultiStepTransform() = default;
  MultiStepTransform(std::initializer_list<Transform> transforms)
      : transforms_(transforms) {}

  [[nodiscard]] const std::vector<Transform>& transforms() const {
    return transforms_;
  }

  bool operator==(const MultiStepTransform& other) const {
    return transforms_ == other.transforms_;
  }

 private:
  std::vector<Transform> transforms_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_MULTISTEPTRANSFORM_H
