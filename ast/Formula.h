#ifndef CHAOSKIT_AST_FORMULA_H
#define CHAOSKIT_AST_FORMULA_H

#include <ostream>
#include "Expression.h"

namespace chaoskit {
namespace ast {

class Formula {
 public:
  Formula(const Expression& x, const Expression& y) : x_(x), y_(y) {}

  template <size_t N>
  Formula(const Expression(&list)[N]) : x_(list[0]), y_(list[1]) {
    static_assert(N == 2, "Formula requires 2 Expressions");
  }

  const Expression& x() const { return x_; }
  const Expression& y() const { return y_; }

  bool operator==(const Formula& other) const {
    return x_ == other.x_ && y_ == other.y_;
  }

 private:
  Expression x_, y_;
};

std::ostream& operator<<(std::ostream& stream, const Formula& formula);

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_AST_FORMULA_H
