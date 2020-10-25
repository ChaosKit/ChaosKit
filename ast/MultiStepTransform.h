#ifndef CHAOSKIT_AST_MULTISTEPTRANSFORM_H
#define CHAOSKIT_AST_MULTISTEPTRANSFORM_H

#include "PositionTransform.h"

namespace chaoskit::ast {

class MultiStepTransform {
 public:
  MultiStepTransform() = default;
  MultiStepTransform(std::initializer_list<PositionTransform> transforms)
      : transforms_(transforms) {}

  [[nodiscard]] const std::vector<PositionTransform>& transforms() const {
    return transforms_;
  }

  bool operator==(const MultiStepTransform& other) const {
    return transforms_ == other.transforms_;
  }

 private:
  std::vector<PositionTransform> transforms_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_MULTISTEPTRANSFORM_H
