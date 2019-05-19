#ifndef CHAOSKIT_CORE_BLEND_H
#define CHAOSKIT_CORE_BLEND_H

#include <memory>
#include <string>
#include <vector>

#include "Formula.h"
#include "Transform.h"
#include "ast/Blend.h"

namespace chaoskit::core {

struct Blend {
  std::vector<std::shared_ptr<Formula>> formulas;
  Transform pre;
  Transform post;
  float weight = 1.f;
  std::string name = "";
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_BLEND_H
