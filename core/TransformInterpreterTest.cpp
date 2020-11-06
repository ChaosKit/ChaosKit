#include <gmock/gmock.h>

#include "TransformInterpreter.h"
#include "ast/helpers.h"
#include "ast/transforms.h"
#include "testing/StaticRng.h"

namespace chaoskit::core {

class TransformInterpreterTest : public testing::Test {};

TEST_F(TransformInterpreterTest, InterpretsAffineTransform) {
  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input = {Point{1, 1}};
  // Scale 2x, then translate by (1, -1)
  TransformParams params;
  params[TransformIndex()] = {2, 0, 1, 0, 2, -1};
  ast::TransformVariant transform = ast::AffineTransform();

  Particle expected{Point{3, 1}};
  ASSERT_EQ(expected, interpreter.interpret(input, transform, params));
}

TEST_F(TransformInterpreterTest, InterpretsFormula) {
  using namespace ast::helpers;

  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input = {Point{1, 1}};
  ast::TransformVariant transform = ast::Formula{
      ast::Input(ast::Input::X) * 2 + 1,
      ast::Input(ast::Input::Y) * 2 - 1,
  };

  Particle expected{Point{3, 1}};
  ASSERT_EQ(expected, interpreter.interpret(input, transform));
}

TEST_F(TransformInterpreterTest, InterpretsMultiStepTransform) {
  using namespace ast::helpers;

  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input = {Point{1, 1}, .5f};
  ast::Transform first(ast::Formula{ast::Input(ast::Input::X) * 2,
                                    ast::Input(ast::Input::Y) * 2},
                       ast::Input(ast::Input::COLOR) + .25f);
  ast::Transform second(ast::Formula{ast::Input(ast::Input::X) + 1,
                                     ast::Input(ast::Input::Y) - 1},
                        ast::Input(ast::Input::COLOR) + .25f);
  ast::TransformVariant transform = ast::MultiStepTransform{first, second};

  Particle expected{Point{3, 1}, 1.f};
  ASSERT_EQ(expected, interpreter.interpret(input, transform));
}

TEST_F(TransformInterpreterTest, InterpretsRandomChoiceTransform) {
  using namespace ast::helpers;

  auto rng = std::make_shared<StaticRng>();
  TransformInterpreter interpreter(rng);
  Particle input = {Point{1, 1}, .5f};
  ast::Transform first(ast::Formula{ast::Input(ast::Input::X) * 2,
                                    ast::Input(ast::Input::Y) * 2},
                       ast::Input(ast::Input::COLOR) + .25f);
  ast::Transform second(ast::Formula{ast::Input(ast::Input::X) + 1,
                                     ast::Input(ast::Input::Y) - 1},
                        ast::Input(ast::Input::COLOR) - .25f);
  ast::TransformVariant transform =
      ast::RandomChoiceTransform{{first, 0.5}, {second, 0.5}};

  Particle expected;
  // First transform
  rng->setDouble(.2);
  expected = {Point{2, 2}, .75f};
  ASSERT_EQ(expected, interpreter.interpret(input, transform));
  // Second transform
  rng->setDouble(.7);
  expected = {Point{2, 0}, .25f};
  ASSERT_EQ(expected, interpreter.interpret(input, transform));
}

TEST_F(TransformInterpreterTest, InterpretsWeightedSumTransform) {
  using namespace ast::helpers;

  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input = {Point{1, 1}, .5f};
  ast::Transform first(ast::Formula{ast::Input(ast::Input::X) * 2,
                                    ast::Input(ast::Input::Y) * 2},
                       ast::Input(ast::Input::COLOR) + .25f);
  ast::Transform second(ast::Formula{ast::Input(ast::Input::X) + 1,
                                     ast::Input(ast::Input::Y) - 1},
                        ast::Input(ast::Input::COLOR) - .25f);
  ast::TransformVariant transform =
      ast::WeightedSumTransform{{first, 3}, {second, 1}};

  Particle actual = interpreter.interpret(input, transform);

  Particle expected{Point{2, 1.5}, .625f};
  ASSERT_EQ(expected, actual);
}

}  // namespace chaoskit::core
