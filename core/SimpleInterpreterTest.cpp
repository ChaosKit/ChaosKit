#include <gmock/gmock.h>

#include "SimpleInterpreter.h"
#include "ast/helpers.h"
#include "core/errors.h"
#include "transforms.h"

namespace chaoskit::core {

using ast::helpers::make_system;
using testing::Eq;

class StaticRng : public Rng {
 public:
  StaticRng() : f_(0), i_(0) {}
  explicit StaticRng(float f) : f_(f), i_(0) {}
  explicit StaticRng(int i) : f_(0), i_(i) {}
  StaticRng(float f, int i) : f_(f), i_(i) {}

  void setFloat(float f) { f_ = f; }
  void setInt(int i) { i_ = i; }

  float randomFloat(float min, float max) override { return f_; }
  int randomInt(int min, int max) override { return i_; }

 private:
  float f_;
  int i_;
};

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
  ast::Expression coloringMethod = ast::Output(ast::Output::Type::X);
  ast::LimitedBlend blend{
      {{ast::WeightedFormula(formula)}, {}, {}, coloringMethod}, 1.f};
  ast::System system{{blend}};

  SimpleInterpreter interpreter(system);

  auto output = make_immortal_particle({1.f, 0.f}, 1.f);
  ASSERT_THAT(interpreter(input),
              Eq(SimpleInterpreter::Result{output, output}));
}

TEST_F(SimpleInterpreterTest, AppliesInitialTransform) {
  ast::System system{};
  std::shared_ptr<Rng> rng = std::make_shared<StaticRng>();
  Transform initialTransform = translate(1, 1);

  SimpleInterpreter interpreter(system, Particle::IMMORTAL, {},
                                initialTransform, rng);

  auto particle = interpreter.randomizeParticle();

  ASSERT_THAT(particle.point, Eq(Point{1, 1}));
}

}  // namespace chaoskit::core
