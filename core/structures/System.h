#ifndef CHAOSKIT_CORE_SYSTEM_H
#define CHAOSKIT_CORE_SYSTEM_H

#include <vector>
#include "Blend.h"

namespace chaoskit::core {

struct System {
  std::vector<Blend*> blends;
  FinalBlend* finalBlend;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEM_H
