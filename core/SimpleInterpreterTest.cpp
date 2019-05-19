#include <gmock/gmock.h>

#include <ast/ast.h>
#include <ast/helpers.h>
#include "SimpleInterpreter.h"

namespace chaoskit::core {

using ast::helpers::make_system;
using testing::Eq;

TEST(SimpleInterpreterTest, EmptySystem) {
  ast::System system{};
  Point input{0.f, 0.f};

  SimpleInterpreter interpreter(system);

  ASSERT_THAT(interpreter(input), Eq(SimpleInterpreter::Result{input, input}));
}

TEST(SimpleInterpreter, InterpretsFormula) {
  ast::Formula formula{1.f, 2.f};

  SimpleInterpreter interpreter(make_system(formula));

  ASSERT_THAT(interpreter({0.f, 0.f}),
              Eq(SimpleInterpreter::Result{Point(1.f, 2.f), Point(1.f, 2.f)}));
}

}  // namespace chaoskit::core
