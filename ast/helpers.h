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

inline Expression n(float value) { return Expression(value); }

inline UnaryFunction sin(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::SIN, expression);
}

inline UnaryFunction cos(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::COS, expression);
}

inline UnaryFunction tan(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::TAN, expression);
}

inline UnaryFunction sqrt(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::SQRT, expression);
}

inline UnaryFunction atan(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::ATAN, expression);
}

inline UnaryFunction trunc(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::TRUNC, expression);
}

inline UnaryFunction exp(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::EXP, expression);
}

inline UnaryFunction floor(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::FLOOR, expression);
}

inline UnaryFunction ceil(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::CEIL, expression);
}

inline UnaryFunction signum(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::SIGNUM, expression);
}

inline UnaryFunction abs(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::ABS, expression);
}

inline UnaryFunction operator-(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::MINUS, expression);
}

inline UnaryFunction negative(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::MINUS, expression);
}

inline UnaryFunction operator!(const Expression& expression) {
  return UnaryFunction(UnaryFunction_Type::NOT, expression);
}

inline BinaryFunction pow(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::POWER, first, second);
}

inline BinaryFunction distance(const Expression& first,
                               const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::DISTANCE, first, second);
}

inline BinaryFunction eq(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::EQUALS, first, second);
}

inline BinaryFunction lt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::LESS_THAN, first, second);
}

inline BinaryFunction gt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::GREATER_THAN, first, second);
}

inline BinaryFunction lte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::LESS_THAN_OR_EQUAL, first, second);
}

inline BinaryFunction gte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::GREATER_THAN_OR_EQUAL, first,
                        second);
}

inline BinaryFunction plus(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::ADD, first, second);
}

inline BinaryFunction operator+(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::ADD, first, second);
}

inline BinaryFunction operator-(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::SUBTRACT, first, second);
}

inline BinaryFunction subtract(const Expression& first,
                               const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::SUBTRACT, first, second);
}

inline BinaryFunction operator*(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::MULTIPLY, first, second);
}

inline BinaryFunction operator/(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::DIVIDE, first, second);
}

inline BinaryFunction operator&&(const Expression& first,
                                 const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::AND, first, second);
}

inline BinaryFunction operator||(const Expression& first,
                                 const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::OR, first, second);
}

inline BinaryFunction operator%(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction_Type::MODULO, first, second);
}

struct InputHelper {
  Input x() const { return Input(Input_Type::X); }
  Input y() const { return Input(Input_Type::Y); }
};

struct ParameterHelper {
  Parameter operator[](size_t index) const { return Parameter(index); }
};

static System make_system(const Formula& f) {
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
