#ifndef CHAOSKIT_CORE_EXPRESSIONINTERPRETER_H
#define CHAOSKIT_CORE_EXPRESSIONINTERPRETER_H

#include <memory>
#include <unordered_map>
#include <vector>
#include "Particle.h"
#include "Rng.h"
#include "ast/Expression.h"
#include "stdx/optional.h"

namespace chaoskit::core {

class ExpressionInterpreter {
 public:
  struct State {
    Particle input;
    stdx::optional<Particle> output;
    std::vector<float> params;
    std::unordered_map<std::string, float> variables;
  };

  explicit ExpressionInterpreter(std::shared_ptr<Rng> rng)
      : rng_(std::move(rng)) {}

  [[nodiscard]] float interpret(const ast::Expression& expression,
                                const State& state) const;

 private:
  std::shared_ptr<Rng> rng_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_EXPRESSIONINTERPRETER_H
