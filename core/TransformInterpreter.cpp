#include "TransformInterpreter.h"

#include <numeric>
#include <utility>

#include "ExpressionInterpreter.h"
#include "Particle.h"
#include "ast/AffineTransform.h"
#include "ast/Formula.h"
#include "ast/MultiStepTransform.h"
#include "ast/RandomChoiceTransform.h"
#include "ast/WeightedSumTransform.h"
#include "ast/util.h"

namespace chaoskit::core {

namespace {

class TransformVisitor {
 public:
  TransformVisitor(Particle input, const TransformParams& params,
                   std::shared_ptr<Rng> rng)
      : input_(input), params_(params), rng_(std::move(rng)), index_() {}

  Particle operator()(const ast::AffineTransform&) const {
    const auto& params = params_.at(index_);
    Particle output = input_;
    output.point = {
        params[0] * input_.x() + params[1] * input_.y() + params[2],
        params[3] * input_.x() + params[4] * input_.y() + params[5]};
    return output;
  }

  Particle operator()(const ast::Formula& formula) const {
    ExpressionInterpreter interpreter(rng_);
    ExpressionInterpreter::State state{input_};
    auto it = params_.find(index_);
    if (it != params_.end()) {
      state.params = &it->second;
    }

    // Evaluate variables first.
    for (const auto& var : formula.variables()) {
      float value = interpreter.interpret(var.definition(), state);
      state.variables.emplace(var.name(), value);
    }

    // Evaluate the formula.
    Particle output = input_;
    output.point = {
        interpreter.interpret(formula.x(), state),
        interpreter.interpret(formula.y(), state),
    };
    return output;
  }

  Particle operator()(const ast::MultiStepTransform& transform) {
    Particle oldInput = input_;
    index_ = index_.firstChild();
    for (const ast::Transform& step : transform.transforms()) {
      input_ = applyTransform(step);
      index_ = index_.nextSibling();
    }
    index_ = index_.parent();

    // Restore the old input and return the "new input" as the output.
    std::swap(input_, oldInput);
    return oldInput;
  }

  Particle operator()(const ast::RandomChoiceTransform& transform) {
    if (transform.transforms().empty()) {
      return input_;
    }

    // Calculate the upper bound of weights.
    double max = 0;
    for (const auto& t : transform.transforms()) {
      max += t.weight;
    }

    // Choose a number smaller between the bounds.
    double choice = rng_->randomDouble(0, max);

    // Find the right transform and apply it.
    double sum = 0.f;
    index_ = index_.firstChild();
    for (const auto& t : transform.transforms()) {
      sum += t.weight;
      if (choice <= sum) {
        Particle output = applyTransform(t.transform);
        index_ = index_.parent();
        return output;
      }
      index_ = index_.nextSibling();
    }

    throw std::out_of_range("Random choice went out of range");
  }

  Particle operator()(const ast::WeightedSumTransform& transform) {
    if (transform.transforms().empty()) {
      return input_;
    }

    // Add the outputs together.
    Particle output{Point{0, 0}, 0};
    index_ = index_.firstChild();
    for (const auto& t : transform.transforms()) {
      Particle singleOutput = applyTransform(t.transform);
      output.point += {static_cast<float>(singleOutput.x() * t.weight),
                       static_cast<float>(singleOutput.y() * t.weight)};
      output.color += static_cast<float>(singleOutput.color * t.weight);
      index_ = index_.nextSibling();
    }
    index_ = index_.parent();
    return output;
  }

  [[nodiscard]] Particle applyTransform(const ast::Transform& transform) {
    // Apply the actual transform variant.
    Particle output = ast::apply_visitor(*this, transform.variant());

    // Calculate the color.
    ExpressionInterpreter interpreter(rng_);
    ExpressionInterpreter::State colorState{input_, output};
    auto it = params_.find(index_);
    if (it != params_.end()) {
      colorState.params = &it->second;
    }
    output.color = interpreter.interpret(transform.color(), colorState);

    return output;
  }

 private:
  Particle input_;
  const TransformParams& params_;
  std::shared_ptr<Rng> rng_;
  TransformIndex index_;
};

}  // namespace

Particle TransformInterpreter::interpret(Particle input,
                                         const ast::Transform& transform,
                                         const TransformParams& params) const {
  return TransformVisitor(input, params, rng_).applyTransform(transform);
}

}  // namespace chaoskit::core
