#include "ExpressionInterpreter.h"

#include <cmath>
#include <unordered_map>

#include "ThreadLocalRng.h"
#include "ast/util.h"
#include "errors.h"

namespace chaoskit::core {

namespace {

using UnaryFn = ast::UnaryFunction::Type;
using BinaryFn = ast::BinaryFunction::Type;

const std::unordered_map<char, std::function<float(float)>> UNARY_FUNCTIONS{
    {UnaryFn::SIN, sinf},
    {UnaryFn::COS, cosf},
    {UnaryFn::TAN, tanf},
    {UnaryFn::MINUS, std::negate<>()},
    {UnaryFn::SQRT, sqrtf},
    {UnaryFn::ATAN, atanf},
    {UnaryFn::TRUNC, truncf},
    {UnaryFn::EXP, expf},
    {UnaryFn::FLOOR, floorf},
    {UnaryFn::CEIL, ceilf},
    {UnaryFn::SIGNUM, [](float f) { return std::signbit(f) ? -1 : 1; }},
    {UnaryFn::ABS, fabsf},
    {UnaryFn::NOT, std::logical_not<>()},
    {UnaryFn::FRAC,
     [](float f) {
       float unused;
       return modff(f, &unused);
     }},
    {UnaryFn::SINH, sinhf},
    {UnaryFn::COSH, coshf},
};

const std::unordered_map<char, std::function<float(float, float)>>
    BINARY_FUNCTIONS{
        {BinaryFn::ADD, std::plus<>()},
        {BinaryFn::SUBTRACT, std::minus<>()},
        {BinaryFn::MULTIPLY, std::multiplies<>()},
        {BinaryFn::DIVIDE, std::divides<>()},
        {BinaryFn::POWER, powf},
        {BinaryFn::MODULO, fmodf},
        {BinaryFn::AND, std::logical_and<>()},
        {BinaryFn::OR, std::logical_or<>()},
        {BinaryFn::LESS_THAN, std::less<>()},
        {BinaryFn::GREATER_THAN, std::greater<>()},
        {BinaryFn::EQUALS, std::equal_to<>()},
        {BinaryFn::LESS_THAN_OR_EQUAL, std::less_equal<>()},
        {BinaryFn::GREATER_THAN_OR_EQUAL, std::greater_equal<>()},
        {BinaryFn::DISTANCE, [](float a, float b) { return std::abs(a - b); }},
        {BinaryFn::ATAN2, atan2f},
    };

class InterpreterVisitor {
 public:
  InterpreterVisitor(const ExpressionInterpreter::State& state,
                     std::shared_ptr<Rng> rng)
      : state_(state), rng_(std::move(rng)) {}

  float operator()(float number) const { return number; }

  float operator()(const ast::Input& input) const {
    switch (input.type()) {
      case ast::Input::X:
        return state_.input.x();
      case ast::Input::Y:
        return state_.input.y();
      case ast::Input::COLOR:
        return state_.input.color;
    }
  }

  float operator()(const ast::Output& output) const {
    if (!state_.output) {
      throw OutputNotAvailable();
    }

    switch (output.type()) {
      case ast::Output::X:
        return state_.output->x();
      case ast::Output::Y:
        return state_.output->y();
    }
  }

  float operator()(const ast::Parameter& param) const {
    try {
      return state_.params.at(param.index());
    } catch (std::out_of_range& e) {
      throw MissingParameterError(param.index());
    }
  }

  float operator()(const ast::VariableName& var) const {
    try {
      return state_.variables.at(var.name());
    } catch (std::out_of_range& e) {
      throw UndefinedVariableError(var.name());
    }
  }

  float operator()(const ast::RandomNumber&) const {
    return rng_->randomFloat(0, 1);
  }

  float operator()(const ast::UnaryFunction& fn) const {
    float argument = ast::apply_visitor(*this, fn.argument());
    return UNARY_FUNCTIONS.at(fn.type())(argument);
  }

  float operator()(const ast::BinaryFunction& fn) const {
    float first = ast::apply_visitor(*this, fn.first());
    float second = ast::apply_visitor(*this, fn.second());
    return BINARY_FUNCTIONS.at(fn.type())(first, second);
  }

 private:
  const ExpressionInterpreter::State& state_;
  std::shared_ptr<Rng> rng_;
};

}  // namespace

float ExpressionInterpreter::interpret(
    const ast::Expression& expression,
    const ExpressionInterpreter::State& state) const {
  return ast::apply_visitor(InterpreterVisitor(state, rng_), expression);
}

}  // namespace chaoskit::core
