#ifndef CHAOSKIT_CORE_BLEND_H
#define CHAOSKIT_CORE_BLEND_H

#include <ast/Blend.h>
#include <vector>

#include "Formula.h"
#include "Transform.h"

namespace chaoskit {
namespace core {

struct Blend {
  Transform pre, post;
  std::vector<Formula> formulas;
  float weight = 1.f;
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_BLEND_H
