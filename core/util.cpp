#include <ast/ast.h>

#include "util.h"

namespace chaoskit::core {

stdx::optional<size_t> formulaIndex(const ast::Formula &formula,
                                    const ast::Blend &blend) {
  const auto &formulas = blend.formulas();
  auto iterator =
      std::find_if(formulas.begin(), formulas.end(),
                   [formula](const ast::WeightedFormula &weightedFormula) {
                     return weightedFormula.formula() == formula;
                   });

  if (iterator == formulas.end()) {
    return stdx::nullopt;
  }
  return static_cast<size_t>(std::distance(formulas.begin(), iterator));
}

stdx::optional<SystemIndex> formulaIndex(const ast::Formula &formula,
                                         const ast::System &system) {
  const auto &blends = system.blends();

  for (size_t i = 0; i < blends.size(); i++) {
    auto index = formulaIndex(formula, blends[i].blend());
    if (index) {
      return SystemIndex{i, *index};
    }
  }

  auto final_index = formulaIndex(formula, system.final_blend());
  if (final_index) {
    return SystemIndex{SystemIndex::FINAL_BLEND, *final_index};
  }

  return stdx::nullopt;
}

}  // namespace chaoskit::core
