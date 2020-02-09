#include <gmock/gmock.h>

#include "SimpleInterpreter.h"
#include "ast/helpers.h"
#include "core/errors.h"

namespace chaoskit::core {

using ast::helpers::make_system;
using testing::Eq;

class SimpleInterpreterTest : public testing::Test {};

Particle make_immortal_particle(Point point, float color = 0.5f) {
  return {point, color, Particle::IMMORTAL};
}
Particle make_particle(Point point, int ttl) { return {point, 0.5f, ttl}; }

TEST_F(SimpleInterpreterTest, EmptySystem) {
  ast::System system{};
  auto input = make_immortal_particle({0.f, 0.f});

  SimpleInterpreter interpreter(system);

  ASSERT_THAT(interpreter(input), Eq(SimpleInterpreter::Result{input, input}));
}

TEST_F(SimpleInterpreterTest, InterpretsFormula) {
  auto input = make_immortal_particle({0.f, 0.f});
  ast::Formula formula{1.f, 2.f};

  SimpleInterpreter interpreter(make_system(formula));

  auto output = make_immortal_particle({1.f, 2.f});
  ASSERT_THAT(interpreter(input),
              Eq(SimpleInterpreter::Result{output, output}));
}

TEST_F(SimpleInterpreterTest, UpdatesColor) {
  auto input = make_immortal_particle({0.f, 0.f}, 0.0f);
  ast::LimitedBlend blend{{{}, {}, {}, 0.1f}, 1.f};
  ast::System system{{blend}};

  SimpleInterpreter interpreter(system);

  auto output = make_immortal_particle({0.f, 0.f}, 0.1f);
  ASSERT_THAT(interpreter(input),
              Eq(SimpleInterpreter::Result{output, output}));
}

TEST_F(SimpleInterpreterTest, DecrementsTtl) {
  ast::System system{};
  auto input = make_particle({0.f, 0.f}, 5);

  SimpleInterpreter interpreter(system);

  auto output = make_particle({0.f, 0.f}, 4);
  ASSERT_THAT(interpreter(input),
              Eq(SimpleInterpreter::Result{output, output}));
}

TEST_F(SimpleInterpreterTest, UsesOutput) {
  auto input = make_immortal_particle({0.f, 0.f}, 0.f);
  ast::Formula formula{1.f, 0.f};
  ast::Expression coloringMethod = ast::Output(ast::Output_Type::X);
  ast::LimitedBlend blend{
      {{ast::WeightedFormula(formula)}, {}, {}, coloringMethod}, 1.f};
  ast::System system{{blend}};

  SimpleInterpreter interpreter(system);

  auto output = make_immortal_particle({1.f, 0.f}, 1.f);
  ASSERT_THAT(interpreter(input),
              Eq(SimpleInterpreter::Result{output, output}));
}

}  // namespace chaoskit::core
