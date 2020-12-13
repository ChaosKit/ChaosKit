#ifndef CHAOSKIT_CORE_TRANSFORMINTERPRETER_H
#define CHAOSKIT_CORE_TRANSFORMINTERPRETER_H

#include <stdx/optional.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Particle.h"
#include "Rng.h"
#include "TransformIndex.h"
#include "TransformParams.h"
#include "ast/transforms.h"

namespace chaoskit::core {

class TransformInterpreter {
 public:
  explicit TransformInterpreter(std::shared_ptr<Rng> rng)
      : rng_(std::move(rng)) {}

  [[nodiscard]] Particle interpret(Particle input,
                                   const ast::Transform& transform,
                                   const TransformParams& params = {}) const;

 private:
  std::shared_ptr<Rng> rng_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TRANSFORMINTERPRETER_H
