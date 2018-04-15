#ifndef CHAOSKIT_AST_FORMULA_H
#define CHAOSKIT_AST_FORMULA_H

#include "Expression.h"
#include "util.h"

namespace chaoskit {
namespace ast {

class Formula {
 public:
  Formula(const Expression& x, const Expression& y) : x_(x), y_(y) {}

  const Expression& x() const { return x_; }
  const Expression& y() const { return y_; }

  bool operator==(const Formula& other) {
    return x_ == other.x_ && y_ == other.y_;
  }

 private:
  Expression x_, y_;
};

GENERATE_NODE_TYPE(Formula)

std::ostream& operator<<(std::ostream& stream, const Formula& formula) {
  StreamPrinter printer(stream);
  stream << node_type(formula) << "{";
  apply_visitor(printer, formula.x());
  stream << ", ";
  apply_visitor(printer, formula.y());
  return stream << "}";
}

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_AST_FORMULA_H
