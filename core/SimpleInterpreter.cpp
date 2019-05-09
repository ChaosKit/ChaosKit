#include "SimpleInterpreter.h"

#include <ast/ast.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

#include "ThreadLocalRng.h"

namespace chaoskit::core {

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

class BlendInterpreter {
 public:
  BlendInterpreter(Point input, const Params &params, size_t blend_index)
      : input_(input), params_(params), index_{blend_index, 0} {}

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
    return params_.at(index_).at(param.index());
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
        index_.formula++;
      }
      input_ = point;
    }

    input_ = (*this)(blend.post());
    return input_;
  }

 private:
  Point input_;
  SystemIndex index_;
  const Params &params_;
};

}  // namespace

SimpleInterpreter::SimpleInterpreter(ast::System system, Params params,
                                     std::shared_ptr<Rng> rng)
    : system_(std::move(system)),
      params_(std::move(params)),
      rng_(std::move(rng)) {
  updateMaxLimit();
}

SimpleInterpreter::SimpleInterpreter(ast::System system, Params params)
    : SimpleInterpreter(std::move(system), std::move(params),
                        std::make_shared<ThreadLocalRng>()) {}

void SimpleInterpreter::updateMaxLimit() {
  max_limit_ = system_.blends().empty() ? 0 : system_.blends().back().limit();
}

void SimpleInterpreter::setSystem(const ast::System &system) {
  system_ = system;
  updateMaxLimit();
}

void SimpleInterpreter::setParams(Params params) {
  params_ = std::move(params);
}

SimpleInterpreter::Result SimpleInterpreter::operator()(Point input) {
  Point next_state = input;

  if (!system_.blends().empty()) {
    float limit = rng_->randomFloat(0.f, max_limit_);
    auto blend_iterator = std::lower_bound(
        system_.blends().begin(), system_.blends().end(), limit,
        [](const ast::LimitedBlend &blend, float limit) {
          return blend.limit() < limit;
        });
    auto blend_index = static_cast<size_t>(
        std::distance(system_.blends().begin(), blend_iterator));

    next_state =
        BlendInterpreter(input, params_, blend_index)(blend_iterator->blend());
  }
  return {next_state,
          BlendInterpreter(next_state, params_,
                           SystemIndex::FINAL_BLEND)(system_.final_blend())};
}

}  // namespace chaoskit::core
