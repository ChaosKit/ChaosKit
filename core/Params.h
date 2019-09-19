#ifndef CHAOSKIT_CORE_PARAMS_H
#define CHAOSKIT_CORE_PARAMS_H

#include <unordered_map>
#include <vector>

#include "SystemIndex.h"
#include "structures/System.h"

namespace chaoskit::core {

class Params {
  std::unordered_map<SystemIndex, std::vector<float>> values_;

 public:
  static Params fromSystem(const System &system) {
    Params result;

    for (size_t i = 0; i < system.blends.size(); ++i) {
      const auto &blend = system.blends[i];
      for (size_t j = 0; j < blend->formulas.size(); ++j) {
        const auto &formula = blend->formulas[j];
        if (!formula->params.empty()) {
          result[SystemIndex{i, j}] = formula->params;
        }
      }
    }

    if (!system.finalBlend) {
      return result;
    }

    for (size_t j = 0; j < system.finalBlend->formulas.size(); ++j) {
      const auto &formula = system.finalBlend->formulas[j];
      if (!formula->params.empty()) {
        result[SystemIndex{SystemIndex::FINAL_BLEND, j}] = formula->params;
      }
    }

    return result;
  }

  std::vector<float> &operator[](const SystemIndex &index) {
    return values_[index];
  }
  std::vector<float> &operator[](SystemIndex &&index) { return values_[index]; }
  std::vector<float> &at(const SystemIndex &index) { return values_.at(index); }
  [[nodiscard]] const std::vector<float> &at(const SystemIndex &index) const {
    return values_.at(index);
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_PARAMS_H
