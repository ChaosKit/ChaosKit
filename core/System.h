#ifndef CHAOSKIT_CORE_SYSTEM_H
#define CHAOSKIT_CORE_SYSTEM_H

#include <ast/System.h>
#include <numeric>
#include <vector>
#include "Blend.h"
#include "Params.h"

namespace chaoskit {
namespace core {

struct System {
  std::vector<Blend> blends;
  Blend final_blend;

  ast::System toSource() const {
    std::vector<float> limits(blends.size());

    std::transform(blends.begin(), blends.end(), limits.begin(),
                   std::mem_fn(&Blend::weight));
    std::partial_sum(limits.begin(), limits.end(), limits.begin());

    std::vector<ast::LimitedBlend> limitedBlends;
    limitedBlends.reserve(blends.size());
    std::transform(blends.begin(), blends.end(), limits.begin(),
                   std::back_inserter(limitedBlends),
                   [](const Blend &blend, float limit) {
                     return ast::LimitedBlend{blend.toSource(), limit};
                   });

    return ast::System{std::move(limitedBlends), final_blend.toSource()};
  }

  Params params() const {
    Params result;

    for (size_t i = 0; i < blends.size(); ++i) {
      const auto &blend = blends[i];
      for (size_t j = 0; j < blend.formulas.size(); ++j) {
        const auto &formula = blend.formulas[j];
        if (!formula.params.empty()) {
          result[SystemIndex{i, j}] = formula.params;
        }
      }
    }

    for (size_t j = 0; j < final_blend.formulas.size(); ++j) {
      const auto &formula = final_blend.formulas[j];
      if (!formula.params.empty()) {
        result[SystemIndex{SystemIndex::FINAL_BLEND, j}] = formula.params;
      }
    }

    return result;
  }
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_SYSTEM_H
