#ifndef CHAOSKIT_AST_FORMULA_H
#define CHAOSKIT_AST_FORMULA_H

#include <ostream>
#include <utility>
#include <vector>
#include "Expression.h"
#include "VariableDeclaration.h"

namespace chaoskit::ast {

class Formula {
 public:
  Formula()
      : x_(Input(Input::Type::X)), y_(Input(Input::Type::Y)), variables_() {}
  Formula(Expression x, Expression y,
          std::vector<VariableDeclaration> variables = {})
      : x_(std::move(x)), y_(std::move(y)), variables_(std::move(variables)) {}

  explicit Formula(const Expression (&list)[2])
      : x_(list[0]), y_(list[1]), variables_() {}

  [[nodiscard]] const Expression& x() const { return x_; }
  [[nodiscard]] const Expression& y() const { return y_; }
  [[nodiscard]] const std::vector<VariableDeclaration>& variables() const {
    return variables_;
  }

  bool operator==(const Formula& other) const {
    return x_ == other.x_ && y_ == other.y_ && variables_ == other.variables_;
  }

 private:
  Expression x_, y_;
  std::vector<VariableDeclaration> variables_;
};

std::ostream& operator<<(std::ostream& stream, const Formula& formula);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_FORMULA_H
