#ifndef CHAOSKIT_AST_BASEMULTITRANSFORM_H
#define CHAOSKIT_AST_BASEMULTITRANSFORM_H

#include "Transform.h"

namespace chaoskit::ast {

class BaseMultiTransform {
 public:
  BaseMultiTransform() = default;

  explicit BaseMultiTransform(std::vector<Transform> transforms)
      : transforms_(std::move(transforms)) {}
  explicit BaseMultiTransform(std::vector<Transform>&& transforms)
      : transforms_(transforms) {}
  explicit BaseMultiTransform(const std::vector<TransformVariant>& variants) {
    for (const auto& variant : variants) {
      transforms_.emplace_back(variant);
    }
  }
  BaseMultiTransform(std::initializer_list<Transform> transforms)
      : transforms_(transforms) {}
  BaseMultiTransform(std::initializer_list<TransformVariant> variants) {
    for (const auto& variant : variants) {
      transforms_.emplace_back(variant);
    }
  }

  [[nodiscard]] const std::vector<Transform>& transforms() const {
    return transforms_;
  }

  [[nodiscard]] size_t size() const noexcept { return transforms_.size(); }
  [[nodiscard]] bool empty() const noexcept { return transforms_.empty(); }

  bool operator==(const BaseMultiTransform& other) const {
    return transforms_ == other.transforms_;
  }

 protected:
  std::vector<Transform> transforms_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_BASEMULTITRANSFORM_H
