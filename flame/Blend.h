#ifndef CHAOSKIT_FLAME_BLEND_H
#define CHAOSKIT_FLAME_BLEND_H

#include <string>
#include <vector>
#include "ColoringMethod.h"
#include "Formula.h"
#include "Transform.h"

namespace chaoskit::flame {

struct BlendBase {
  std::vector<Formula*> formulas;
  Transform pre;
  Transform post;
  ColoringMethod coloringMethod;
  bool enabled = true;
};

struct Blend : public BlendBase {
  float weight = 1.f;
  std::string name = "";
};

struct FinalBlend : public BlendBase {};

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_BLEND_H
