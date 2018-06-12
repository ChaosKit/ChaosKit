#include <gmock/gmock.h>

#include "../ast/ast.h"
#include "../ast/helpers.h"
#include "SimpleInterpreter.h"

namespace chaoskit {
namespace core {

using ast::Formula;
using ast::System;
using ast::helpers::make_system;
using testing::Eq;

TEST(SimpleInterpreterTest, EmptySystem) {
  System system{};
  Point input{0.f, 0.f};

  SimpleInterpreter interpreter(system, input);

  ASSERT_THAT(interpreter.step(), Eq(input));
}

TEST(SimpleInterpreter, InterpretsFormula) {
  Formula formula{1.f, 2.f};

  SimpleInterpreter interpreter(make_system(formula));

  ASSERT_THAT(interpreter.step(), Eq(Point(1.f, 2.f)));
}

}  // namespace core
}  // namespace chaoskit
