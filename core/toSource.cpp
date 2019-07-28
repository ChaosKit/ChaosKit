#include "toSource.h"

#include <cmath>
#include <numeric>
#include <vector>

namespace chaoskit::core {

ast::Blend toSource(const BlendBase &blend) {
  std::vector<ast::WeightedFormula> weighted_formulas;
  weighted_formulas.reserve(blend.formulas.size());
  for (const auto &formula : blend.formulas) {
    weighted_formulas.push_back(toSource(*formula));
  }

  return ast::Blend{std::move(weighted_formulas), toSource(blend.pre),
                    toSource(blend.post)};
}

ast::WeightedFormula toSource(const Formula &formula) {
  return ast::WeightedFormula{formula.source, formula.weight.x,
                              formula.weight.y};
}

ast::Transform toSource(const Transform &transform) {
  return ast::Transform(transform.values);
}

ast::System toSource(const System &system) {
  // Generate a monotonically increasing vector of limits
  std::vector<float> limits(system.blends.size());
  std::transform(system.blends.begin(), system.blends.end(), limits.begin(),
                 [](const Blend *blend) { return blend->weight; });
  std::partial_sum(limits.begin(), limits.end(), limits.begin());

  // Combine blends and limits into LimitedBlends
  std::vector<ast::LimitedBlend> limitedBlends;
  limitedBlends.reserve(system.blends.size());
  std::transform(system.blends.begin(), system.blends.end(), limits.begin(),
                 std::back_inserter(limitedBlends),
                 [](const Blend *blend, float limit) {
                   return ast::LimitedBlend{toSource(*blend), limit};
                 });

  return ast::System{std::move(limitedBlends), toSource(*system.finalBlend)};
}

}  // namespace chaoskit::core
