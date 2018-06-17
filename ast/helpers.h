#ifndef CHAOSKIT_AST_HELPERS_H
#define CHAOSKIT_AST_HELPERS_H

#include <cstddef>
#include "BinaryFunction.h"
#include "Expression.h"
#include "Input.h"
#include "Parameter.h"
#include "System.h"
#include "UnaryFunction.h"

namespace chaoskit {
namespace ast {
namespace helpers {

Expression n(float value) { return Expression(value); }

UnaryFunction sin(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::SIN, expression);
}

UnaryFunction cos(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::COS, expression);
}

UnaryFunction tan(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::TAN, expression);
}

UnaryFunction sqrt(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::SQRT, expression);
}

UnaryFunction atan(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::ATAN, expression);
}

UnaryFunction trunc(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::TRUNC, expression);
}

UnaryFunction exp(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::EXP, expression);
}

UnaryFunction floor(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::FLOOR, expression);
}

UnaryFunction ceil(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::CEIL, expression);
}

UnaryFunction signum(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::SIGNUM, expression);
}

UnaryFunction abs(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::ABS, expression);
}

UnaryFunction operator-(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::MINUS, expression);
}

UnaryFunction negative(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::MINUS, expression);
}

UnaryFunction operator!(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::NOT, expression);
}

BinaryFunction pow(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::POWER, first, second);
}

BinaryFunction distance(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::DISTANCE, first, second);
}

BinaryFunction eq(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::EQUALS, first, second);
}

BinaryFunction lt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::LESS_THAN, first, second);
}

BinaryFunction gt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::GREATER_THAN, first, second);
}

BinaryFunction lte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::LESS_THAN_OR_EQUAL, first, second);
}

BinaryFunction gte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::GREATER_THAN_OR_EQUAL, first,
                        second);
}

BinaryFunction operator+(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::ADD, first, second);
}

BinaryFunction operator-(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::SUBTRACT, first, second);
}

BinaryFunction subtract(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::SUBTRACT, first, second);
}

BinaryFunction operator*(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::MULTIPLY, first, second);
}

BinaryFunction operator/(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::DIVIDE, first, second);
}

BinaryFunction operator&&(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::AND, first, second);
}

BinaryFunction operator||(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::OR, first, second);
}

BinaryFunction operator%(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::MODULO, first, second);
}

struct InputHelper {
  Input x() const { return Input(Input_Type::X); }
  Input y() const { return Input(Input_Type::Y); }
};

struct ParameterHelper {
  Parameter operator[](size_t index) const { return Parameter(index); }
};

System make_system(const Formula& f) {
  return System{{LimitedBlend(Blend{{WeightedFormula(f)}}, 1.f)}};
}

struct Helper {
  InputHelper input;
  ParameterHelper params;
};

}  // namespace helpers
}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_AST_HELPERS_H
