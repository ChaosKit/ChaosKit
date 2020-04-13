#ifndef CHAOSKIT_CORE_SYSTEM_H
#define CHAOSKIT_CORE_SYSTEM_H

#include <vector>
#include "Blend.h"
#include "Transform.h"

namespace chaoskit::core {

struct System {
  std::vector<Blend*> blends;
  FinalBlend* finalBlend;

  // Lifetime of all particles in this system.
  int32_t ttl = 20;

  // Applied immediately after randomizing a particle. Use if you want to change
  // the default randomization bounds from (-1; 1) on both axes.
  Transform initialTransform;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEM_H
