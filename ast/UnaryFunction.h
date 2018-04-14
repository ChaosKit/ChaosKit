#ifndef CHAOSKIT_UNARYFUNCTION_H
#define CHAOSKIT_UNARYFUNCTION_H

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
  Expression argument() const { return argument_; }

 private:
  Type type_;
  Expression argument_;
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_UNARYFUNCTION_H
