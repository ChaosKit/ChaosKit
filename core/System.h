#ifndef CHAOSKIT_CORE_SYSTEM_H
#define CHAOSKIT_CORE_SYSTEM_H

#include <ast/System.h>
#include <numeric>
#include <vector>
#include "Blend.h"

namespace chaoskit {
namespace core {

struct System {
  std::vector<Blend> blends;
  Blend final_blend;

  ast::System toSource() const {
    std::vector<float> limits;
    limits.reserve(blends.size());
    std::partial_sum(
        blends.begin(), blends.end(), std::back_inserter(limits),
        [](const Blend &a, const Blend &b) { return a.weight + b.weight; });

    std::vector<ast::LimitedBlend> limitedBlends;
    limitedBlends.reserve(blends.size());
    std::transform(blends.begin(), blends.end(), limits.begin(),
                   std::back_inserter(limitedBlends),
                   [](const Blend &blend, float limit) {
                     return ast::LimitedBlend{blend.toSource(), limit};
                   });

    return ast::System{std::move(limitedBlends), final_blend.toSource()};
  }
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_SYSTEM_H
