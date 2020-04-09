#ifndef CHAOSKIT_AST_FORMULA_H
#define CHAOSKIT_AST_FORMULA_H

#include <ostream>
#include <utility>
#include "Expression.h"

namespace chaoskit::ast {

class Formula {
 public:
  Formula() : x_(Input(Input::Type::X)), y_(Input(Input::Type::Y)) {}
  Formula(Expression x, Expression y) : x_(std::move(x)), y_(std::move(y)) {}

  explicit Formula(const Expression (&list)[2]) : x_(list[0]), y_(list[1]) {}

  [[nodiscard]] const Expression& x() const { return x_; }
  [[nodiscard]] const Expression& y() const { return y_; }

  bool operator==(const Formula& other) const {
    return x_ == other.x_ && y_ == other.y_;
  }

 private:
  Expression x_, y_;
};

std::ostream& operator<<(std::ostream& stream, const Formula& formula);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_FORMULA_H
