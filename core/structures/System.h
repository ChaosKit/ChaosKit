#ifndef CHAOSKIT_CORE_SYSTEM_H
#define CHAOSKIT_CORE_SYSTEM_H

#include <ast/System.h>
#include <memory>
#include <numeric>
#include <vector>

#include "Blend.h"

namespace chaoskit::core {

struct System {
  std::vector<std::shared_ptr<Blend>> blends;
  std::shared_ptr<Blend> final_blend;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEM_H
