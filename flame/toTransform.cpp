#include "toTransform.h"

#include <vector>

#include "ast/transforms.h"

namespace chaoskit::flame {

ast::Transform toTransform(const BlendBase& blend) {
  std::vector<ast::Transform> transforms;
  std::vector<double> weights;

  for (const auto* formula : blend.formulas) {
    transforms.emplace_back(formula->source);
    weights.push_back(formula->weight.x);
  }

  return ast::Transform(
      ast::MultiStepTransform{
          ast::AffineTransform{},  // pre
          ast::WeightedSumTransform(std::move(transforms), std::move(weights)),
          ast::AffineTransform{}},  // post
      blend.coloringMethod.source);
}

ast::Transform toTransform(const System& system) {
  if (system.isolatedBlend) {
    return toTransform(*system.isolatedBlend);
  }

  std::vector<ast::Transform> transforms;
  std::vector<double> weights;
  for (const Blend* blend : system.blends) {
    if (!blend->enabled) continue;
    transforms.emplace_back(toTransform(*blend));
    weights.push_back(blend->weight);
  }
  return ast::Transform(
      ast::RandomChoiceTransform(std::move(transforms), std::move(weights)));
}

stdx::optional<ast::Transform> toCameraTransform(const System& system) {
  if (system.finalBlend && system.finalBlend->enabled) {
    return toTransform(*system.finalBlend);
  }

  return stdx::nullopt;
}

}  // namespace chaoskit::flame
