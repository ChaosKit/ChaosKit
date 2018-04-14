#ifndef CHAOSKIT_BINARYFUNCTION_H
#define CHAOSKIT_BINARYFUNCTION_H

#include <enum.h>
#include "Expression.h"

namespace chaoskit {
namespace ast {

BETTER_ENUM(BinaryFunction_Type, char, ADD, SUBTRACT, MULTIPLY, DIVIDE, POWER,
            MODULO, AND, OR, LESS_THAN, GREATER_THAN, EQUALS,
            LESS_THAN_OR_EQUAL, GREATER_THAN_OR_EQUAL, DISTANCE)

class BinaryFunction {
 public:
  typedef BinaryFunction_Type Type;

  BinaryFunction(Type type, const Expression& first, const Expression& second)
      : type_(type), first_(first), second_(second) {}

  Type type() const { return type_; }
  Expression first() const { return first_; }
  Expression second() const { return second_; }

 private:
  Type type_;
  Expression first_, second_;
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_BINARYFUNCTION_H
