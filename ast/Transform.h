#ifndef CHAOSKIT_AST_TRANSFORM_H
#define CHAOSKIT_AST_TRANSFORM_H

#include "Expression.h"
#include "PositionTransform.h"

namespace chaoskit::ast {

class Transform {
 public:
  Transform() = delete;
  explicit Transform(PositionTransform position,
                     Expression color = Input(Input::COLOR))
      : position_(std::move(position)), color_(std::move(color)) {}

  [[nodiscard]] const PositionTransform& position() const { return position_; }
  [[nodiscard]] const Expression& color() const { return color_; }

  bool operator==(const Transform& other) const {
    return position_ == other.position_ && color_ == other.color_;
  }

 private:
  PositionTransform position_;
  Expression color_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_TRANSFORM_H
