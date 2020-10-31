#include <gmock/gmock.h>
#include <cmath>

#include "ExpressionInterpreter.h"
#include "errors.h"
#include "testing/StaticRng.h"

// [[nodiscard]] issues warnings in ASSERT_THROW tests
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

namespace chaoskit::core {

using namespace ast;
using State = ExpressionInterpreter::State;

Particle makeImmortalParticle(Point point, float color = 0.5f) {
  return {point, color, Particle::IMMORTAL};
}

class ExpressionInterpreterTest : public testing::Test {};

TEST_F(ExpressionInterpreterTest, InterpretsConstant) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  Expression expression = 42.f;

  float actual =
      interpreter.interpret(expression, State{makeImmortalParticle({})});

  ASSERT_EQ(42.f, actual);
}

TEST_F(ExpressionInterpreterTest, InterpretsInput) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input = makeImmortalParticle({1.f, 2.f}, 0.25f);

  ASSERT_EQ(1.f, interpreter.interpret(Input(Input::X), State{input}));
  ASSERT_EQ(2.f, interpreter.interpret(Input(Input::Y), State{input}));
  ASSERT_EQ(.25f, interpreter.interpret(Input(Input::COLOR), State{input}));
}

TEST_F(ExpressionInterpreterTest, InterpretsOutput) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input = makeImmortalParticle({});
  Particle output = makeImmortalParticle({1.f, 2.f}, 0.25f);

  ASSERT_EQ(1.f,
            interpreter.interpret(Output(Output::X), State{input, output}));
  ASSERT_EQ(2.f,
            interpreter.interpret(Output(Output::Y), State{input, output}));
}

TEST_F(ExpressionInterpreterTest, ThrowsWhenOutputIsMissing) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  Particle input = makeImmortalParticle({});

  ASSERT_THROW(interpreter.interpret(Output(Output::X), State{input}),
               OutputNotAvailable);
}

TEST_F(ExpressionInterpreterTest, InterpretsParameter) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;
  state.params = {1, 2};

  ASSERT_EQ(1.f, interpreter.interpret(Parameter(0), state));
  ASSERT_EQ(2.f, interpreter.interpret(Parameter(1), state));
}

TEST_F(ExpressionInterpreterTest, ThrowsWhenParameterIsMissing) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;
  state.params = {1, 2};

  ASSERT_THROW(interpreter.interpret(Parameter(2), state),
               MissingParameterError);
}

TEST_F(ExpressionInterpreterTest, InterpretsVariableName) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;
  state.variables = {{"foo", 42.f}};

  ASSERT_EQ(42.f, interpreter.interpret(VariableName("foo"), state));
}

TEST_F(ExpressionInterpreterTest, ThrowsWhenVariableIsUndefined) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;
  state.variables = {{"foo", 42.f}};

  ASSERT_THROW(interpreter.interpret(VariableName("bar"), state),
               UndefinedVariableError);
}

TEST_F(ExpressionInterpreterTest, InterpretsRandomNumber) {
  auto rng = std::make_shared<StaticRng>();
  rng->setFloat(.25f);
  ExpressionInterpreter interpreter(rng);

  ASSERT_EQ(.25f, interpreter.interpret(RandomNumber(), {}));
}

class UnaryFnInterpreterTest : public ExpressionInterpreterTest,
                               public testing::WithParamInterface<float> {};

INSTANTIATE_TEST_SUITE_P(Parameterized, UnaryFnInterpreterTest,
                         testing::Range(-.5f, .5f, 0.1f));

TEST_P(UnaryFnInterpreterTest, InterpretsSin) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_EQ(sinf(GetParam()),
            interpreter.interpret(UnaryFunction(UnaryFunction::SIN, GetParam()),
                                  state));
}

TEST_P(UnaryFnInterpreterTest, InterpretsCos) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_EQ(cosf(GetParam()),
            interpreter.interpret(UnaryFunction(UnaryFunction::COS, GetParam()),
                                  state));
}

TEST_P(UnaryFnInterpreterTest, InterpretsTan) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_EQ(tanf(GetParam()),
            interpreter.interpret(UnaryFunction(UnaryFunction::TAN, GetParam()),
                                  state));
}

TEST_P(UnaryFnInterpreterTest, InterpretsMinus) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_EQ(-GetParam(),
            interpreter.interpret(
                UnaryFunction(UnaryFunction::MINUS, GetParam()), state));
}

TEST_P(UnaryFnInterpreterTest, InterpretSqrt) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::SQRT, GetParam()), state),
              testing::NanSensitiveFloatEq(sqrt(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretAtan) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::ATAN, GetParam()), state),
              testing::FloatEq(atan(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretTrunc) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::TRUNC, GetParam()), state),
              testing::FloatEq(trunc(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretExp) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::EXP, GetParam()), state),
              testing::FloatEq(exp(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretFloor) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::FLOOR, GetParam()), state),
              testing::FloatEq(floor(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretCeil) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::CEIL, GetParam()), state),
              testing::FloatEq(ceil(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretSignum) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  float expected = GetParam() < 0.f ? -1.f : 1.f;
  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::SIGNUM, GetParam()), state),
              testing::FloatEq(expected));
}

TEST_P(UnaryFnInterpreterTest, InterpretAbs) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::ABS, GetParam()), state),
              testing::FloatEq(abs(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretNot) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::NOT, GetParam()), state),
              testing::FloatEq(!GetParam()));
}

TEST_P(UnaryFnInterpreterTest, InterpretFrac) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  float unused;
  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::FRAC, GetParam()), state),
              testing::FloatEq(modf(GetParam(), &unused)));
}

TEST_P(UnaryFnInterpreterTest, InterpretSinh) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::SINH, GetParam()), state),
              testing::FloatEq(sinh(GetParam())));
}

TEST_P(UnaryFnInterpreterTest, InterpretCosh) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  ASSERT_THAT(interpreter.interpret(
                  UnaryFunction(UnaryFunction::COSH, GetParam()), state),
              testing::FloatEq(cosh(GetParam())));
}

class BinaryFnInterpreterTest
    : public ExpressionInterpreterTest,
      public testing::WithParamInterface<std::pair<float, float>> {};

INSTANTIATE_TEST_SUITE_P(Parameterized, BinaryFnInterpreterTest,
                         testing::Values(std::make_pair(1.f, 0.5f),
                                         std::make_pair(2.f, 3.f),
                                         std::make_pair(4.f, 4.f)));

TEST_P(BinaryFnInterpreterTest, InterpretAdd) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::ADD, first, second), state),
              testing::FloatEq(first + second));
}

TEST_P(BinaryFnInterpreterTest, InterpretSubtract) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(
      interpreter.interpret(
          BinaryFunction(BinaryFunction::SUBTRACT, first, second), state),
      testing::FloatEq(first - second));
}

TEST_P(BinaryFnInterpreterTest, InterpretMultiply) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(
      interpreter.interpret(
          BinaryFunction(BinaryFunction::MULTIPLY, first, second), state),
      testing::FloatEq(first * second));
}

TEST_P(BinaryFnInterpreterTest, InterpretDivide) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::DIVIDE, first, second), state),
              testing::FloatEq(first / second));
}

TEST_P(BinaryFnInterpreterTest, InterpretPower) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::POWER, first, second), state),
              testing::FloatEq(pow(first, second)));
}

TEST_P(BinaryFnInterpreterTest, InterpretModulo) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::MODULO, first, second), state),
              testing::FloatEq(fmod(first, second)));
}

TEST_P(BinaryFnInterpreterTest, InterpretAnd) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::AND, first, second), state),
              testing::Eq(first && second));
}

TEST_P(BinaryFnInterpreterTest, InterpretOr) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::OR, first, second), state),
              testing::Eq(first || second));
}

TEST_P(BinaryFnInterpreterTest, InterpretLessThan) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(
      interpreter.interpret(
          BinaryFunction(BinaryFunction::LESS_THAN, first, second), state),
      testing::Eq(first < second));
}

TEST_P(BinaryFnInterpreterTest, InterpretGreaterThan) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(
      interpreter.interpret(
          BinaryFunction(BinaryFunction::GREATER_THAN, first, second), state),
      testing::Eq(first > second));
}

TEST_P(BinaryFnInterpreterTest, InterpretEquals) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::EQUALS, first, second), state),
              testing::Eq(first == second));
}

TEST_P(BinaryFnInterpreterTest, InterpretLessThanOrEqual) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(
      interpreter.interpret(
          BinaryFunction(BinaryFunction::LESS_THAN_OR_EQUAL, first, second),
          state),
      testing::Eq(first <= second));
}

TEST_P(BinaryFnInterpreterTest, InterpretGreaterThanOrEqual) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(
      interpreter.interpret(
          BinaryFunction(BinaryFunction::GREATER_THAN_OR_EQUAL, first, second),
          state),
      testing::Eq(first >= second));
}

TEST_P(BinaryFnInterpreterTest, InterpretDistance) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  float expected = abs(first - second);
  ASSERT_THAT(
      interpreter.interpret(
          BinaryFunction(BinaryFunction::DISTANCE, first, second), state),
      testing::FloatEq(expected));
}

TEST_P(BinaryFnInterpreterTest, InterpretAtan2) {
  ExpressionInterpreter interpreter(std::make_shared<StaticRng>());
  State state;

  auto [first, second] = GetParam();
  ASSERT_THAT(interpreter.interpret(
                  BinaryFunction(BinaryFunction::ATAN2, first, second), state),
              testing::FloatEq(atan2(first, second)));
}

}  // namespace chaoskit::core

#pragma GCC diagnostic pop
