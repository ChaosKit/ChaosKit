#include "SimpleInterpreter.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

#include "ThreadLocalRng.h"
#include "ast/ast.h"
#include "errors.h"

namespace chaoskit::core {

using UnaryFn = ast::UnaryFunction::Type;
using BinaryFn = ast::BinaryFunction::Type;
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
    {UnaryFn::FRAC, [](float f) {
       float unused;
       return modff(f, &unused);
     }}};

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
  BlendInterpreter(Particle input, const Params &params, size_t blend_index)
      : input_(input),
        output_(input),
        params_(params),
        index_{blend_index, 0} {}

  float operator()(float number) const { return number; }

  float operator()(const ast::Input &input) const {
    switch (input.type()) {
      case ast::Input::Type::X:
        return input_.x();
      case ast::Input::Type::Y:
        return input_.y();
      case ast::Input::Type::COLOR:
        return input_.color;
    }
  }

  float operator()(const ast::Output &output) const {
    switch (output.type()) {
      case ast::Output::Type::X:
        return output_.x();
      case ast::Output::Type::Y:
        return output_.y();
    }
  }

  float operator()(const ast::Parameter &param) const {
    try {
      return params_.at(index_).at(param.index());
    } catch (std::out_of_range &e) {
      throw MissingParameterError(index_, param.index());
    }
  }

  float operator()(const ast::UnaryFunction &function) const {
    float value = apply_visitor(*this, function.argument());
    return UNARY_FUNCTIONS.at(function.type())(value);
  }

  float operator()(const ast::BinaryFunction &function) const {
    float first = apply_visitor(*this, function.first());
    float second = apply_visitor(*this, function.second());
    return BINARY_FUNCTIONS.at(function.type())(first, second);
  }

  Particle operator()(const ast::Transform &transform) const {
    const auto &point = output_.point;
    const auto &params = transform.params();
    return outputWithPoint(
        Point(params[0] * point.x() + params[1] * point.y() + params[2],
              params[3] * point.x() + params[4] * point.y() + params[5]));
  }

  Particle operator()(const ast::Formula &formula) const {
    return outputWithPoint(Point(apply_visitor(*this, formula.x()),
                                 apply_visitor(*this, formula.y())));
  }

  Particle operator()(const ast::WeightedFormula &formula) const {
    Particle particle((*this)(formula.formula()));
    particle.point = Point(particle.x() * formula.weight_x(),
                           particle.y() * formula.weight_y());
    return particle;
  }

  Particle operator()(const ast::Blend &blend) {
    output_ = (*this)(blend.pre());

    if (!blend.formulas().empty()) {
      Point point;
      for (const auto &formula : blend.formulas()) {
        point += (*this)(formula).point;
        index_.formula++;
      }
      output_.point = point;
    }

    output_ = (*this)(blend.post());
    index_.formula = SystemIndex::COLORING_METHOD;
    output_.color = apply_visitor(*this, blend.coloringMethod());
    return output_;
  }

 private:
  Particle input_, output_;
  SystemIndex index_;
  const Params &params_;

  [[nodiscard]] Particle outputWithPoint(Point point) const {
    return {point, output_.color, output_.ttl};
  }
};

}  // namespace

SimpleInterpreter::SimpleInterpreter(ast::System system, int ttl, Params params,
                                     std::shared_ptr<Rng> rng)
    : system_(std::move(system)),
      ttl_(ttl),
      params_(std::move(params)),
      rng_(std::move(rng)) {
  updateMaxLimit();
}

SimpleInterpreter::SimpleInterpreter(ast::System system, int ttl, Params params)
    : SimpleInterpreter(std::move(system), ttl, std::move(params),
                        std::make_shared<ThreadLocalRng>()) {}

void SimpleInterpreter::updateMaxLimit() {
  max_limit_ = system_.blends().empty() ? 0 : system_.blends().back().limit();
}

Particle SimpleInterpreter::randomizeParticle() {
  Particle particle;
  randomizeParticle(particle);
  particle.ttl = (ttl_ == Particle::IMMORTAL) ? Particle::IMMORTAL
                                              : rng_->randomInt(1, ttl_);
  return particle;
}

void SimpleInterpreter::randomizeParticle(Particle &particle) {
  particle.point =
      Point(rng_->randomFloat(-1.f, 1.f), rng_->randomFloat(-1.f, 1.f));
  particle.color = rng_->randomFloat(0.f, 1.f);
}

void SimpleInterpreter::setSystem(const ast::System &system) {
  system_ = system;
  updateMaxLimit();
}

void SimpleInterpreter::setParams(Params params) {
  params_ = std::move(params);
}

void SimpleInterpreter::setTtl(int ttl) { ttl_ = ttl; }

SimpleInterpreter::Result SimpleInterpreter::operator()(Particle input) {
  Particle next_state = input;

  if (next_state.ttl == 0) {
    randomizeParticle(next_state);
    next_state.ttl = ttl_;
  }

  if (!system_.blends().empty()) {
    float limit = rng_->randomFloat(0.f, max_limit_);
    auto blend_iterator = std::lower_bound(
        system_.blends().begin(), system_.blends().end(), limit,
        [](const ast::LimitedBlend &blend, float limit) {
          return blend.limit() < limit;
        });
    auto blend_index = static_cast<size_t>(
        std::distance(system_.blends().begin(), blend_iterator));

    next_state = BlendInterpreter(next_state, params_,
                                  blend_index)(blend_iterator->blend());
  }

  if (next_state.ttl != Particle::IMMORTAL) {
    --next_state.ttl;
  }

  Particle output = BlendInterpreter(
      next_state, params_, SystemIndex::FINAL_BLEND)(system_.final_blend());

  return {next_state, output};
}

}  // namespace chaoskit::core
