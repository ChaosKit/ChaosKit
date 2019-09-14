#include "toSource.h"

#include <cmath>
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
  std::vector<ast::LimitedBlend> limitedBlends;
  float currentLimit = 0.f;

  for (const core::Blend *blend : system.blends) {
    if (!blend->enabled) continue;
    currentLimit += blend->weight;
    limitedBlends.emplace_back(toSource(*blend), currentLimit);
  }

  if (system.finalBlend->enabled) {
    return ast::System{std::move(limitedBlends), toSource(*system.finalBlend)};
  } else {
    return ast::System{std::move(limitedBlends)};
  }
}

}  // namespace chaoskit::core
