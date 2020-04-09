#ifndef CHAOSKIT_AST_UNARYFUNCTION_H
#define CHAOSKIT_AST_UNARYFUNCTION_H

#include <ostream>
#include <utility>
#include "Expression.h"

namespace chaoskit::ast {

class UnaryFunction {
 public:
  enum Type : char {
    SIN,
    COS,
    TAN,
    MINUS,
    SQRT,
    ATAN,
    TRUNC,
    EXP,
    FLOOR,
    CEIL,
    SIGNUM,
    ABS,
    NOT,
    FRAC,
  };

  UnaryFunction(Type type, Expression argument)
      : type_(type), argument_(std::move(argument)) {}

  [[nodiscard]] Type type() const { return type_; }
  [[nodiscard]] const Expression& argument() const { return argument_; }

  bool operator==(const UnaryFunction& other) const {
    return type_ == other.type_ && argument_ == other.argument_;
  }

 private:
  Type type_;
  Expression argument_;
};

std::ostream& operator<<(std::ostream& stream, const UnaryFunction& function);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_UNARYFUNCTION_H
