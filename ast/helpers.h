#ifndef CHAOSKIT_AST_HELPERS_H
#define CHAOSKIT_AST_HELPERS_H

#include <cstddef>
#include "Input.h"
#include "Parameter.h"
#include "UnaryFunction.h"
#include "BinaryFunction.h"
#include "Expression.h"

namespace chaoskit {
namespace ast {
namespace helpers {

Expression n(float value) {
  return Expression(value);
}

UnaryFunction sin(const Expression& expression) {
  return UnaryFunction(UnaryFunction::SIN, expression);
}

UnaryFunction cos(const Expression& expression) {
  return UnaryFunction(UnaryFunction::COS, expression);
}

UnaryFunction tan(const Expression& expression) {
  return UnaryFunction(UnaryFunction::TAN, expression);
}

UnaryFunction sqrt(const Expression& expression) {
  return UnaryFunction(UnaryFunction::SQRT, expression);
}

UnaryFunction atan(const Expression& expression) {
  return UnaryFunction(UnaryFunction::ATAN, expression);
}

UnaryFunction trunc(const Expression& expression) {
  return UnaryFunction(UnaryFunction::TRUNC, expression);
}

UnaryFunction exp(const Expression& expression) {
  return UnaryFunction(UnaryFunction::EXP, expression);
}

UnaryFunction floor(const Expression& expression) {
  return UnaryFunction(UnaryFunction::FLOOR, expression);
}

UnaryFunction ceil(const Expression& expression) {
  return UnaryFunction(UnaryFunction::CEIL, expression);
}

UnaryFunction signum(const Expression& expression) {
  return UnaryFunction(UnaryFunction::SIGNUM, expression);
}

UnaryFunction abs(const Expression& expression) {
  return UnaryFunction(UnaryFunction::ABS, expression);
}

UnaryFunction operator-(const Expression& expression) {
  return UnaryFunction(UnaryFunction::MINUS, expression);
}

UnaryFunction operator!(const Expression& expression) {
  return UnaryFunction(UnaryFunction::NOT, expression);
}

BinaryFunction pow(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::POWER, first, second);
}

BinaryFunction distance(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::DISTANCE, first, second);
}

BinaryFunction eq(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::EQUALS, first, second);
}

BinaryFunction lt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::LESS_THAN, first, second);
}

BinaryFunction gt(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::GREATER_THAN, first, second);
}

BinaryFunction lte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::LESS_THAN_OR_EQUAL, first, second);
}

BinaryFunction gte(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::GREATER_THAN_OR_EQUAL, first, second);
}

BinaryFunction operator+(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::ADD, first, second);
}

BinaryFunction operator-(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::SUBTRACT, first, second);
}

BinaryFunction operator*(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::MULTIPLY, first, second);
}

BinaryFunction operator/(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::DIVIDE, first, second);
}

BinaryFunction operator&&(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::AND, first, second);
}

BinaryFunction operator||(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::OR, first, second);
}

BinaryFunction operator%(const Expression& first, const Expression& second) {
  return BinaryFunction(BinaryFunction::MODULO, first, second);
}

struct InputHelper {
  Input x() const {
    return Input(Input::X);
  }
  Input y() const {
    return Input(Input::Y);
  }
};

struct ParameterHelper {
  Parameter operator[](size_t index) const {
    return Parameter(index);
  }
};

struct Helper {
  InputHelper input;
  ParameterHelper params;
};

}
}
}

#endif //CHAOSKIT_AST_HELPERS_H
