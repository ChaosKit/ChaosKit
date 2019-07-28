#ifndef CHAOSKIT_CORE_BLEND_H
#define CHAOSKIT_CORE_BLEND_H

#include <string>
#include <vector>
#include "Formula.h"
#include "Transform.h"
#include "state/hierarchy.h"

namespace chaoskit::core {

struct BlendBase {
  std::vector<Formula*> formulas;
  Transform pre;
  Transform post;
};

struct Blend : public BlendBase {
  float weight = 1.f;
  std::string name = "";
};

struct FinalBlend : public BlendBase {};

}  // namespace chaoskit::core

CHAOSKIT_DEFINE_RELATION(::chaoskit::core::Blend, ::chaoskit::core::Formula,
                         &::chaoskit::core::Blend::formulas);
CHAOSKIT_DEFINE_RELATION(::chaoskit::core::FinalBlend,
                         ::chaoskit::core::Formula,
                         &::chaoskit::core::FinalBlend::formulas);
CHAOSKIT_DEFINE_CHILDREN(::chaoskit::core::Blend, ::chaoskit::core::Formula);
CHAOSKIT_DEFINE_CHILDREN(::chaoskit::core::FinalBlend,
                         ::chaoskit::core::Formula);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::core::Formula, ::chaoskit::core::Blend,
                        ::chaoskit::core::FinalBlend);

#endif  // CHAOSKIT_CORE_BLEND_H
