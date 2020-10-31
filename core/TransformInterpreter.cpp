#include "TransformInterpreter.h"

#include <numeric>
#include <utility>

#include "ExpressionInterpreter.h"
#include "ast/AffineTransform.h"
#include "ast/Formula.h"
#include "ast/MultiStepTransform.h"
#include "ast/RandomChoiceTransform.h"
#include "ast/WeightedSumTransform.h"
#include "ast/util.h"

namespace chaoskit::core {

namespace {

class InterpreterVisitor {
 public:
  InterpreterVisitor(TransformInterpreter::State state,
                     std::shared_ptr<Rng> rng)
      : state_(std::move(state)), rng_(std::move(rng)), index_() {}

  Particle operator()(const ast::AffineTransform&) const {
    const auto& params = state_.params.at(index_);
    Particle output = state_.input;
    output.point = {
        params[0] * state_.input.x() + params[1] * state_.input.y() + params[2],
        params[3] * state_.input.x() + params[4] * state_.input.y() +
            params[5]};
    return output;
  }

  Particle operator()(const ast::Formula& formula) const {
    ExpressionInterpreter interpreter(rng_);
    ExpressionInterpreter::State state{state_.input};
    auto it = state_.params.find(index_);
    if (it != state_.params.end()) {
      state.params = it->second;
    }

    // Evaluate variables first.
    for (const auto& var : formula.variables()) {
      float value = interpreter.interpret(var.definition(), state);
      state.variables.emplace(var.name(), value);
    }

    // Evaluate the formula.
    Particle output = state_.input;
    output.point = {
        interpreter.interpret(formula.x(), state),
        interpreter.interpret(formula.y(), state),
    };
    return output;
  }

  Particle operator()(const ast::MultiStepTransform& transform) {
    index_ = index_.firstChild();
    for (const ast::Transform& step : transform.transforms()) {
      state_.input = applyTransform(step);
      index_ = index_.nextSibling();
    }
    index_ = index_.parent();
    return state_.input;
  }

  Particle operator()(const ast::RandomChoiceTransform& transform) {
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
    // Calculate the upper bound of weights.
    double max = 0;
    for (const auto& t : transform.transforms()) {
      max += t.weight;
    }

    // Add the outputs together.
    Particle output{Point{0, 0}, 0};
    index_ = index_.firstChild();
    for (const auto& t : transform.transforms()) {
      double weight = t.weight / max;
      Particle singleOutput = applyTransform(t.transform);
      output.point += {static_cast<float>(singleOutput.x() * weight),
                       static_cast<float>(singleOutput.y() * weight)};
      output.color += {static_cast<float>(singleOutput.color * weight)};
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
    ExpressionInterpreter::State colorState{state_.input, output};
    auto it = state_.params.find(index_);
    if (it != state_.params.end()) {
      colorState.params = it->second;
    }
    output.color = interpreter.interpret(transform.color(), colorState);

    return output;
  }

 private:
  TransformInterpreter::State state_;
  std::shared_ptr<Rng> rng_;
  TransformIndex index_;
};

}  // namespace

Particle TransformInterpreter::interpret(const ast::Transform& transform,
                                         State state) const {
  return InterpreterVisitor(std::move(state), rng_).applyTransform(transform);
}

}  // namespace chaoskit::core
