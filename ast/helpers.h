#ifndef CHAOSKIT_AST_HELPERS_H
#define CHAOSKIT_AST_HELPERS_H

#include <cstddef>
#include "Input.h"
#include "Parameter.h"
#include "UnaryFunction.h"
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
