#ifndef CHAOSKIT_AST_BINARYFUNCTION_H
#define CHAOSKIT_AST_BINARYFUNCTION_H

#include <enum.h>
#include <ostream>
#include "Expression.h"

namespace chaoskit::ast {

BETTER_ENUM(BinaryFunction_Type, char, ADD, SUBTRACT, MULTIPLY, DIVIDE, POWER,
            MODULO, AND, OR, LESS_THAN, GREATER_THAN, EQUALS,
            LESS_THAN_OR_EQUAL, GREATER_THAN_OR_EQUAL, DISTANCE)

class BinaryFunction {
 public:
  typedef BinaryFunction_Type Type;

  BinaryFunction(Type type, const Expression& first, const Expression& second)
      : type_(type), first_(first), second_(second) {}

  Type type() const { return type_; }
  const Expression& first() const { return first_; }
  const Expression& second() const { return second_; }

  bool operator==(const BinaryFunction& other) const {
    return type_ == other.type_ && first_ == other.first_ &&
           second_ == other.second_;
  }

 private:
  Type type_;
  Expression first_, second_;
};

std::ostream& operator<<(std::ostream& stream, const BinaryFunction& function);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_BINARYFUNCTION_H
