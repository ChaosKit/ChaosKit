#ifndef CHAOSKIT_AST_TRANSFORM_H
#define CHAOSKIT_AST_TRANSFORM_H

#include "Expression.h"
#include "TransformVariant.h"

namespace chaoskit::ast {

class Transform {
 public:
  Transform() = delete;
  explicit Transform(TransformVariant variant,
                     Expression color = Input(Input::COLOR))
      : variant_(std::move(variant)), color_(std::move(color)) {}

  [[nodiscard]] const TransformVariant& variant() const { return variant_; }
  [[nodiscard]] const Expression& color() const { return color_; }

  bool operator==(const Transform& other) const {
    return variant_ == other.variant_ && color_ == other.color_;
  }

 private:
  TransformVariant variant_;
  Expression color_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_TRANSFORM_H
