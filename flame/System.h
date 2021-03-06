#ifndef CHAOSKIT_FLAME_SYSTEM_H
#define CHAOSKIT_FLAME_SYSTEM_H

#include <vector>
#include "Blend.h"
#include "Transform.h"

namespace chaoskit::flame {

struct System {
  std::vector<Blend*> blends;
  FinalBlend* finalBlend;

  // If set, this will be the only blend visible. Must be an existing Blend from
  // the blends vector.
  const Blend* isolatedBlend = nullptr;

  // Number of steps that need to pass to start rendering.
  int32_t skip = 0;

  // Lifetime of all particles in this system.
  int32_t ttl = 20;

  // Applied immediately after randomizing a particle. Use if you want to change
  // the default randomization bounds from (-1; 1) on both axes.
  Transform initialTransform{};
};

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_SYSTEM_H
