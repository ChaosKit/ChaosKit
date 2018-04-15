#ifndef CHAOSKIT_UNARYFUNCTION_H
#define CHAOSKIT_UNARYFUNCTION_H

#include <ostream>
#include <enum.h>
#include "Expression.h"

namespace chaoskit {
namespace ast {

BETTER_ENUM(UnaryFunction_Type, char, SIN, COS, TAN, MINUS, SQRT, ATAN, TRUNC,
            EXP, FLOOR, CEIL, SIGNUM, ABS, NOT)

class UnaryFunction {
 public:
  typedef UnaryFunction_Type Type;

  UnaryFunction(Type type, const Expression& argument)
      : type_(type), argument_(argument) {}

  Type type() const { return type_; }
  const Expression& argument() const { return argument_; }

  bool operator==(const UnaryFunction& other) const {
    return type_ == other.type_ && argument_ == other.argument_;
  }

 private:
  Type type_;
  Expression argument_;
};

std::ostream& operator<<(std::ostream& stream, const UnaryFunction& function);

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_UNARYFUNCTION_H
