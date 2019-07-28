#ifndef CHAOSKIT_CORE_SYSTEM_H
#define CHAOSKIT_CORE_SYSTEM_H

#include <vector>
#include "Blend.h"
#include "state/hierarchy.h"

namespace chaoskit::core {

struct System {
  std::vector<Blend*> blends;
  FinalBlend* finalBlend;
};

}  // namespace chaoskit::core

CHAOSKIT_DEFINE_RELATION(::chaoskit::core::System, ::chaoskit::core::Blend,
                         &::chaoskit::core::System::blends);
CHAOSKIT_DEFINE_RELATION(::chaoskit::core::System, ::chaoskit::core::FinalBlend,
                         &::chaoskit::core::System::finalBlend);
CHAOSKIT_DEFINE_CHILDREN(::chaoskit::core::System, ::chaoskit::core::Blend,
                         ::chaoskit::core::FinalBlend);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::core::Blend, ::chaoskit::core::System);
CHAOSKIT_DEFINE_PARENTS(::chaoskit::core::FinalBlend, ::chaoskit::core::System);

#endif  // CHAOSKIT_CORE_SYSTEM_H
