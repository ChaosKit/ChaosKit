#include <gmock/gmock.h>

#include "TransformInterpreter.h"
#include "ast/helpers.h"
#include "ast/transforms.h"
#include "core/debug.h"  // Debug formatters, keep.
#include "testing/StaticRng.h"

namespace chaoskit::core {

class TransformInterpreterTest : public testing::Test {};

TEST_F(TransformInterpreterTest, InterpretsAffineTransform) {
  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input{Point{1, 1}};
  // Scale 2x, then translate by (1, -1)
  TransformParams params;
  params[TransformIndex()] = {2, 0, 1, 0, 2, -1};
  ast::TransformVariant transform = ast::AffineTransform();

  Particle expected{Point{3, 1}};
  ASSERT_EQ(expected, interpreter.interpret(input, transform, params));
}

TEST_F(TransformInterpreterTest, InterpretsRotationsCorrectly) {
  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input{Point{1, 1}};
  // Rotate by 45 degrees counterclockwise.
  constexpr float COS_45DEG = 0.7071067811865476;
  constexpr float SIN_45DEG = 0.7071067811865476;
  TransformParams params;

  params[TransformIndex()] = {COS_45DEG, -SIN_45DEG, 0,
                              SIN_45DEG, COS_45DEG,  0};
  ast::TransformVariant transform = ast::AffineTransform();

  Particle expected{Point{0, M_SQRT2}};
  ASSERT_EQ(expected, interpreter.interpret(input, transform, params));
}

TEST_F(TransformInterpreterTest, InterpretsFormula) {
  using namespace ast::helpers;

  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input{Point{1, 1}};
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
  Particle input{Point{1, 1}, .5f};
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
  Particle input{Point{1, 1}, .5f};
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
  expected = Particle(2, 2, .75f);
  ASSERT_EQ(expected, interpreter.interpret(input, transform));
  // Second transform
  rng->setDouble(.7);
  expected = Particle(2, 0, .25f);
  ASSERT_EQ(expected, interpreter.interpret(input, transform));
}

TEST_F(TransformInterpreterTest, EmptyRandomChoiceReturnsInput) {
  auto rng = std::make_shared<StaticRng>();
  TransformInterpreter interpreter(rng);
  Particle input{Point{1, 1}, .5f};
  ast::TransformVariant transform = ast::RandomChoiceTransform{};

  ASSERT_EQ(input, interpreter.interpret(input, transform));
}

TEST_F(TransformInterpreterTest, InterpretsWeightedSumTransform) {
  using namespace ast::helpers;

  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input{Point{1, 1}, .5f};
  ast::Transform first(ast::Formula{ast::Input(ast::Input::X) * 2,
                                    ast::Input(ast::Input::Y) * 2},
                       ast::Input(ast::Input::COLOR) + .25f);
  ast::Transform second(ast::Formula{ast::Input(ast::Input::X) + 1,
                                     ast::Input(ast::Input::Y) - 1},
                        ast::Input(ast::Input::COLOR) - .25f);
  ast::TransformVariant transform =
      ast::WeightedSumTransform{{first, .75f}, {second, .25f}};

  Particle actual = interpreter.interpret(input, transform);

  Particle expected{Point{2, 1.5}, .625f};
  ASSERT_EQ(expected, actual);
}

TEST_F(TransformInterpreterTest, EmptyWeightedSumReturnsInput) {
  auto rng = std::make_shared<StaticRng>();
  TransformInterpreter interpreter(rng);
  Particle input{Point{1, 1}, .5f};
  ast::TransformVariant transform = ast::WeightedSumTransform{};

  ASSERT_EQ(input, interpreter.interpret(input, transform));
}

TEST_F(TransformInterpreterTest, RestoresInputAfterMultiStep) {
  using namespace ast::helpers;

  // The actual multi-step actually doesn't matter.
  ast::Transform first(ast::Formula{ast::Input(ast::Input::X) * 2,
                                    ast::Input(ast::Input::Y) * 2},
                       ast::Input(ast::Input::COLOR) + .25f);
  ast::Transform second(ast::Formula{ast::Input(ast::Input::X) + 1,
                                     ast::Input(ast::Input::Y) - 1},
                        ast::Input(ast::Input::COLOR) + .25f);
  ast::TransformVariant multiStep = ast::MultiStepTransform{first, second};
  // This is the important step. The weighted sum reads the input color and
  // sets it as its output color.
  // If the multi-step modifies the input itself, the color would be 1.f.
  ast::Transform transform{ast::WeightedSumTransform{{multiStep, 1}},
                           ast::Input(ast::Input::COLOR)};

  TransformInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input{Point{1, 1}, .5f};

  ASSERT_FLOAT_EQ(.5f, interpreter.interpret(input, transform).color);
}

}  // namespace chaoskit::core
