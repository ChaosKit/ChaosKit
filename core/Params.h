#ifndef CHAOSKIT_CORE_PARAMS_H
#define CHAOSKIT_CORE_PARAMS_H

#include <unordered_map>
#include <vector>

#include "flame/Blend.h"
#include "flame/System.h"
#include "flame/SystemIndex.h"

namespace chaoskit::core {

class Params {
  std::unordered_map<flame::SystemIndex, std::vector<float>> values_;

 public:
  static Params fromSystem(const flame::System &system) {
    Params result;

    if (system.isolatedBlend) {
      addBlend(system.isolatedBlend, 0, result);
    } else {
      size_t blendIndex = 0;
      for (const auto &blend : system.blends) {
        if (!blend->enabled) continue;
        addBlend(blend, blendIndex, result);
        blendIndex++;
      }
    }

    if (!system.finalBlend || !system.finalBlend->enabled) {
      return result;
    }

    for (size_t j = 0; j < system.finalBlend->formulas.size(); ++j) {
      const auto &formula = system.finalBlend->formulas[j];
      if (!formula->params.empty()) {
        result[flame::SystemIndex{flame::SystemIndex::FINAL_BLEND, j}] =
            formula->params;
      }
    }

    if (!system.finalBlend->coloringMethod.params.empty()) {
      result[flame::SystemIndex{flame::SystemIndex::FINAL_BLEND,
                                flame::SystemIndex::COLORING_METHOD}] =
          system.finalBlend->coloringMethod.params;
    }

    return result;
  }

  std::vector<float> &operator[](const flame::SystemIndex &index) {
    return values_[index];
  }
  std::vector<float> &operator[](flame::SystemIndex &&index) {
    return values_[index];
  }
  std::vector<float> &at(const flame::SystemIndex &index) {
    return values_.at(index);
  }
  [[nodiscard]] const std::vector<float> &at(
      const flame::SystemIndex &index) const {
    return values_.at(index);
  }

 private:
  static void addBlend(const flame::Blend *blend, size_t blendIndex,
                       Params &result) {
    for (size_t j = 0; j < blend->formulas.size(); ++j) {
      const auto &formula = blend->formulas[j];
      if (!formula->params.empty()) {
        result[flame::SystemIndex{blendIndex, j}] = formula->params;
      }
    }

    if (!blend->coloringMethod.params.empty()) {
      result[flame::SystemIndex{blendIndex,
                                flame::SystemIndex::COLORING_METHOD}] =
          blend->coloringMethod.params;
    }
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_PARAMS_H
