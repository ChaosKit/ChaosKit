#include "TransformInterpreter.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
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
    if (transform.empty()) {
      return input_;
    }

    const auto& weights = transform.weights();
    const size_t count = transform.size();
    double total = 0.0;
    size_t lastPositive = count;
    for (size_t i = 0; i < count; ++i) {
      const double w = (std::max)(0.0, weights[i]);
      if (w > 0.0) {
        total += w;
        lastPositive = i;
      }
    }

    if (total <= 0.0 || lastPositive == count || !std::isfinite(total)) {
      return input_;
    }

    const double choice = rng_->randomDouble(0.0, total);

    double sum = 0.0;
    size_t selected = lastPositive;
    for (size_t i = 0; i < count; ++i) {
      const double w = (std::max)(0.0, weights[i]);
      if (w <= 0.0) {
        continue;
      }
      sum += w;
      if (choice <= sum) {
        selected = i;
        break;
      }
    }

    index_ = index_.firstChild();
    for (size_t i = 0; i < selected; ++i) {
      index_ = index_.nextSibling();
    }
    Particle output = applyTransform(transform.transforms()[selected]);
    index_ = index_.parent();
    return output;
  }

  Particle operator()(const ast::WeightedSumTransform& transform) {
    if (transform.empty()) {
      return input_;
    }

    // Add the outputs together.
    Particle output{Point{0, 0}, 0};
    index_ = index_.firstChild();
    for (int i = 0; i < transform.size(); ++i) {
      Particle singleOutput = applyTransform(transform.transforms()[i]);
      double weight = transform.weights()[i];
      output.point += {static_cast<float>(singleOutput.x() * weight),
                       static_cast<float>(singleOutput.y() * weight)};
      output.color += static_cast<float>(singleOutput.color * weight);
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
