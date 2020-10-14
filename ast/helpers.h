#ifndef CHAOSKIT_AST_HELPERS_H
#define CHAOSKIT_AST_HELPERS_H

#include <cstddef>
#include "BinaryFunction.h"
#include "Expression.h"
#include "Input.h"
#include "Parameter.h"
#include "System.h"
#include "UnaryFunction.h"

namespace chaoskit::ast::helpers {

inline Expression n(float value) { return Expression(value); }

inline Expression v(const char* variableName) {
  return Expression(variableName);
}

inline RandomNumber random() { return RandomNumber(); }

inline UnaryFunction sin(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::SIN, expression);
}

inline UnaryFunction cos(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::COS, expression);
}

inline UnaryFunction tan(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::TAN, expression);
}

inline UnaryFunction sqrt(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::SQRT, expression);
}

inline UnaryFunction atan(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::ATAN, expression);
}

inline UnaryFunction trunc(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::TRUNC, expression);
}

inline UnaryFunction exp(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::EXP, expression);
}

inline UnaryFunction floor(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::FLOOR, expression);
}

inline UnaryFunction ceil(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::CEIL, expression);
}

inline UnaryFunction signum(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::SIGNUM, expression);
}

inline UnaryFunction abs(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::ABS, expression);
}

inline UnaryFunction operator-(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::MINUS, expression);
}

inline UnaryFunction negative(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::MINUS, expression);
}

inline UnaryFunction operator!(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::NOT, expression);
}

inline UnaryFunction frac(const Expression& expression) {
  return UnaryFunction(UnaryFunction::Type::FRAC, expression);
}

inline BinaryFunction pow(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::POWER, first, second);
}

inline BinaryFunction distance(const Expression& first,
                               const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::DISTANCE, first, second);
}

inline BinaryFunction atan2(const Expression& y, const Expression& x) {
  return BinaryFunction(BinaryFunction::Type::ATAN2, y, x);
}

inline BinaryFunction eq(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::EQUALS, first, second);
}

inline BinaryFunction lt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::LESS_THAN, first, second);
}

inline BinaryFunction gt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::GREATER_THAN, first, second);
}

inline BinaryFunction lte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::LESS_THAN_OR_EQUAL, first,
                        second);
}

inline BinaryFunction gte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::GREATER_THAN_OR_EQUAL, first,
                        second);
}

inline BinaryFunction plus(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::ADD, first, second);
}

inline BinaryFunction operator+(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::ADD, first, second);
}

inline BinaryFunction operator-(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::SUBTRACT, first, second);
}

inline BinaryFunction subtract(const Expression& first,
                               const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::SUBTRACT, first, second);
}

inline BinaryFunction operator*(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::MULTIPLY, first, second);
}

inline BinaryFunction operator/(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::DIVIDE, first, second);
}

inline BinaryFunction operator&&(const Expression& first,
                                 const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::AND, first, second);
}

inline BinaryFunction operator||(const Expression& first,
                                 const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::OR, first, second);
}

inline BinaryFunction operator%(const Expression& first,
                                const Expression& second) {
  return BinaryFunction(BinaryFunction::Type::MODULO, first, second);
}

struct InputHelper {
  [[nodiscard]] Input x() const { return Input(Input::Type::X); }
  [[nodiscard]] Input y() const { return Input(Input::Type::Y); }
  [[nodiscard]] Input color() const { return Input(Input::Type::COLOR); }
};

struct OutputHelper {
  [[nodiscard]] Output x() const { return Output(Output::Type::X); }
  [[nodiscard]] Output y() const { return Output(Output::Type::Y); }
};

struct ParameterHelper {
  Parameter operator[](size_t index) const { return Parameter(index); }
};

static System make_system(const Formula& f) {
  return System{{LimitedBlend(Blend{{WeightedFormula(f)}}, 1.f)}};
}

struct Helper {
  InputHelper input;
  OutputHelper output;
  ParameterHelper params;
};

}  // namespace chaoskit::ast::helpers

#endif  // CHAOSKIT_AST_HELPERS_H
