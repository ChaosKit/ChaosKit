#include "toSource.h"

#include <cmath>
#include <vector>
#include "ast/Input.h"

namespace chaoskit::core {

namespace {

std::vector<ast::WeightedFormula> prepareWeightedFormulas(
    const BlendBase &blend) {
  std::vector<ast::WeightedFormula> weightedFormulas;
  weightedFormulas.reserve(blend.formulas.size());

  // Weights have to be normalized to sum to 1.0
  float xSum = 0.f;
  float ySum = 0.f;
  for (const auto &formula : blend.formulas) {
    xSum += formula->weight.x;
    ySum += formula->weight.y;
  }
  for (const auto &formula : blend.formulas) {
    weightedFormulas.emplace_back(formula->source, formula->weight.x / xSum,
                                  formula->weight.y / ySum);
  }
  return weightedFormulas;
}

}  // namespace

ast::Blend toSource(const BlendBase &blend) {
  return ast::Blend{prepareWeightedFormulas(blend), toSource(blend.pre),
                    toSource(blend.post), toSource(blend.coloringMethod)};
}

ast::WeightedFormula toSource(const Formula &formula) {
  return ast::WeightedFormula{formula.source, formula.weight.x,
                              formula.weight.y};
}

ast::Expression toSource(const ColoringMethod &coloringMethod) {
  return coloringMethod.source;
}

ast::Transform toSource(const Transform &transform) {
  return ast::Transform(transform.values);
}

ast::System toSource(const System &system) {
  std::vector<ast::LimitedBlend> limitedBlends;
  float currentLimit = 0.f;

  if (system.isolatedBlend) {
    limitedBlends.emplace_back(toSource(*system.isolatedBlend), 0);
  } else {
    for (const core::Blend *blend : system.blends) {
      if (!blend->enabled) continue;
      currentLimit += blend->weight;
      limitedBlends.emplace_back(toSource(*blend), currentLimit);
    }
  }

  if (system.finalBlend && system.finalBlend->enabled) {
    return ast::System{std::move(limitedBlends), toSource(*system.finalBlend)};
  } else {
    return ast::System{std::move(limitedBlends)};
  }
}

}  // namespace chaoskit::core
