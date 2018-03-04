#ifndef CHAOSKIT_UNARYFUNCTION_H
#define CHAOSKIT_UNARYFUNCTION_H

#include "Expression.h"

namespace chaoskit {
namespace ast {

class UnaryFunction {
 public:
  enum Type {
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
    NOT
  };

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
