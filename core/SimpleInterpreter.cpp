#include "../ast/ast.h"
// ast.h has to be first to satisfy forward declarations
#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>
#include "SimpleInterpreter.h"
#include "ThreadLocalRng.h"

namespace chaoskit {
namespace core {

using UnaryFn = ast::UnaryFunction_Type;
using BinaryFn = ast::BinaryFunction_Type;
using ast::apply_visitor;

namespace {

const std::unordered_map<char, std::function<float(float)>> UNARY_FUNCTIONS{
    {UnaryFn::SIN, sinf},
    {UnaryFn::COS, cosf},
    {UnaryFn::TAN, tanf},
    {UnaryFn::MINUS, std::negate<float>()},
    {UnaryFn::SQRT, sqrtf},
    {UnaryFn::ATAN, atanf},
    {UnaryFn::TRUNC, truncf},
    {UnaryFn::EXP, expf},
    {UnaryFn::FLOOR, floorf},
    {UnaryFn::CEIL, ceilf},
    {UnaryFn::SIGNUM, [](float f) { return std::signbit(f) ? -1 : 1; }},
    {UnaryFn::ABS, fabsf},
    {UnaryFn::NOT, std::logical_not<float>()},
};

const std::unordered_map<char, std::function<float(float, float)>>
    BINARY_FUNCTIONS{
        {BinaryFn::ADD, std::plus<float>()},
        {BinaryFn::SUBTRACT, std::minus<float>()},
        {BinaryFn::MULTIPLY, std::multiplies<float>()},
        {BinaryFn::DIVIDE, std::divides<float>()},
        {BinaryFn::POWER, powf},
        {BinaryFn::MODULO, fmodf},
        {BinaryFn::AND, std::logical_and<float>()},
        {BinaryFn::OR, std::logical_or<float>()},
        {BinaryFn::LESS_THAN, std::less<float>()},
        {BinaryFn::GREATER_THAN, std::greater<float>()},
        {BinaryFn::EQUALS, std::equal_to<float>()},
        {BinaryFn::LESS_THAN_OR_EQUAL, std::less_equal<float>()},
        {BinaryFn::GREATER_THAN_OR_EQUAL, std::greater_equal<float>()},
        {BinaryFn::DISTANCE, [](float a, float b) { return std::abs(a - b); }},
    };

class InterpreterImpl {
 public:
  InterpreterImpl(Rng *rng, Point input, float max_limit,
                  const std::vector<float> &params)
      : rng_(rng), input_(input), max_limit_(max_limit), params_(params) {}

  float operator()(float number) const { return number; }

  float operator()(const ast::Input &input) const {
    switch (input.type()) {
      case ast::Input_Type::X:
        return input_.x();
      case ast::Input_Type::Y:
        return input_.y();
    }
  }

  float operator()(const ast::Parameter &param) const {
    return params_.at(param.index());
  }

  float operator()(const ast::UnaryFunction &function) const {
    float value = apply_visitor(*this, function.argument());
    return UNARY_FUNCTIONS.at(function.type()._to_integral())(value);
  }

  float operator()(const ast::BinaryFunction &function) const {
    float first = apply_visitor(*this, function.first());
    float second = apply_visitor(*this, function.second());
    return BINARY_FUNCTIONS.at(function.type()._to_integral())(first, second);
  }

  Point operator()(const ast::Transform &transform) const {
    const auto &params = transform.params();
    return Point(params[0] * input_.x() + params[1] * input_.y() + params[2],
                 params[3] * input_.x() + params[4] * input_.y() + params[5]);
  }

  Point operator()(const ast::Formula &formula) const {
    return Point(apply_visitor(*this, formula.x()),
                 apply_visitor(*this, formula.y()));
  }

  Point operator()(const ast::WeightedFormula &formula) const {
    Point point((*this)(formula.formula()));

    return Point(point.x() * formula.weight_x(),
                 point.y() * formula.weight_y());
  }

  Point operator()(const ast::Blend &blend) {
    input_ = (*this)(blend.pre());

    if (!blend.formulas().empty()) {
      Point point;
      for (const auto &formula : blend.formulas()) {
        point += (*this)(formula);
      }
      input_ = point;
    }

    input_ = (*this)(blend.post());
    return input_;
  }

  Point operator()(const ast::System &system) {
    if (system.blends().empty()) {
      return input_;
    }

    float limit = rng_->randomFloat(0.f, max_limit_);
    const auto &blend =
        std::lower_bound(system.blends().begin(), system.blends().end(), limit,
                         [](const ast::LimitedBlend &blend, float limit) {
                           return blend.limit() < limit;
                         })
            ->blend();

    return (*this)(blend);
  }

 private:
  Rng *rng_;
  Point input_;
  float max_limit_;
  const std::vector<float> &params_;
};

}  // namespace

SimpleInterpreter::SimpleInterpreter(ast::System system,
                                     std::vector<float> params,
                                     std::shared_ptr<Rng> rng)
    : system_(std::move(system)), params_(std::move(params)), rng_(rng) {
  updateMaxLimit();
}

SimpleInterpreter::SimpleInterpreter(ast::System system,
                                     std::vector<float> params)
    : SimpleInterpreter(std::move(system), std::move(params),
                        std::make_shared<ThreadLocalRng>()) {}

void SimpleInterpreter::updateMaxLimit() {
  max_limit_ = system_.blends().empty() ? 0 : system_.blends().back().limit();
}

void SimpleInterpreter::setSystem(const ast::System &system) {
  system_ = system;
  updateMaxLimit();
}

void SimpleInterpreter::setParams(const std::vector<float> &params) {
  params_ = params;
}

SimpleInterpreter::Result SimpleInterpreter::step(Point input) {
  InterpreterImpl interpreter(rng_.get(), input, max_limit_, params_);
  Point next_state = interpreter(system_);
  return {next_state, interpreter(system_.final_blend())};
}

}  // namespace core
}  // namespace chaoskit
