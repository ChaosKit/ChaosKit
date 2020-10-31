#ifndef CHAOSKIT_CORE_TRANSFORMINTERPRETER_H
#define CHAOSKIT_CORE_TRANSFORMINTERPRETER_H

#include <stdx/optional.h>
#include <unordered_map>
#include <vector>
#include "Particle.h"
#include "Rng.h"
#include "TransformIndex.h"
#include "ast/Transform.h"

namespace chaoskit::core {

class TransformInterpreter {
 public:
  using Params = std::unordered_map<TransformIndex, std::vector<float>>;

  struct State {
    Particle input;
    Params params;
  };

  explicit TransformInterpreter(std::shared_ptr<Rng> rng)
      : rng_(std::move(rng)) {}

  [[nodiscard]] Particle interpret(const ast::Transform& transform,
                                   State state) const;

 private:
  std::shared_ptr<Rng> rng_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TRANSFORMINTERPRETER_H
