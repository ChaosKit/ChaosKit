#ifndef CHAOSKIT_CORE_BLEND_H
#define CHAOSKIT_CORE_BLEND_H

#include <ast/Blend.h>
#include <memory>
#include <vector>

#include "Formula.h"
#include "Transform.h"

namespace chaoskit::core {

struct Blend {
  Transform pre, post;
  std::vector<std::shared_ptr<Formula>> formulas;
  float weight = 1.f;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_BLEND_H
