#ifndef CHAOSKIT_CORE_SYSTEM_H
#define CHAOSKIT_CORE_SYSTEM_H

#include <ast/System.h>
#include <numeric>
#include <vector>
#include "Blend.h"
#include "Params.h"

namespace chaoskit::core {

struct System {
  std::vector<Blend> blends;
  Blend final_blend;

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

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEM_H
