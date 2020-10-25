#ifndef CHAOSKIT_AST_BINARYFUNCTION_H
#define CHAOSKIT_AST_BINARYFUNCTION_H

#include <utility>
#include "Expression.h"

namespace chaoskit::ast {

class BinaryFunction {
 public:
  enum Type : char {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    POWER,
    MODULO,
    AND,
    OR,
    LESS_THAN,
    GREATER_THAN,
    EQUALS,
    LESS_THAN_OR_EQUAL,
    GREATER_THAN_OR_EQUAL,
    DISTANCE,
    ATAN2,
  };

  BinaryFunction(Type type, Expression first, Expression second)
      : type_(type), first_(std::move(first)), second_(std::move(second)) {}

  [[nodiscard]] Type type() const { return type_; }
  [[nodiscard]] const Expression& first() const { return first_; }
  [[nodiscard]] const Expression& second() const { return second_; }

  bool operator==(const BinaryFunction& other) const {
    return type_ == other.type_ && first_ == other.first_ &&
           second_ == other.second_;
  }

 private:
  Type type_;
  Expression first_, second_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_BINARYFUNCTION_H
