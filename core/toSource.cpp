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
  std::vector<core::Blend *> enabledBlends;
  std::copy_if(system.blends.begin(), system.blends.end(),
               std::back_inserter(enabledBlends),
               [](const Blend *blend) { return blend->enabled; });

  // Generate a monotonically increasing vector of limits
  std::vector<float> limits(enabledBlends.size());
  std::transform(enabledBlends.begin(), enabledBlends.end(), limits.begin(),
                 [](const Blend *blend) { return blend->weight; });
  std::partial_sum(limits.begin(), limits.end(), limits.begin());

  // Combine blends and limits into LimitedBlends
  std::vector<ast::LimitedBlend> limitedBlends;
  limitedBlends.reserve(enabledBlends.size());
  std::transform(enabledBlends.begin(), enabledBlends.end(), limits.begin(),
                 std::back_inserter(limitedBlends),
                 [](const Blend *blend, float limit) {
                   return ast::LimitedBlend{toSource(*blend), limit};
                 });

  if (system.finalBlend->enabled) {
    return ast::System{std::move(limitedBlends), toSource(*system.finalBlend)};
  } else {
    return ast::System{std::move(limitedBlends)};
  }
}

}  // namespace chaoskit::core
