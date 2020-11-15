#include "toTransform.h"

#include <vector>

#include "ast/transforms.h"

namespace chaoskit::flame {

ast::Transform toTransform(const BlendBase& blend) {
  std::vector<ast::TransformWithWeight> formulas;
  for (const auto* formula : blend.formulas) {
    formulas.emplace_back(ast::Transform(formula->source), formula->weight.x);
  }

  return ast::Transform(
      ast::MultiStepTransform{
          ast::Transform(ast::AffineTransform{}),  // pre
          ast::Transform(ast::WeightedSumTransform(std::move(formulas))),
          ast::Transform(ast::AffineTransform{})},  // post
      blend.coloringMethod.source);
}

ast::Transform toTransform(const System& system) {
  if (system.isolatedBlend) {
    return toTransform(*system.isolatedBlend);
  }

  std::vector<ast::TransformWithWeight> blends;
  for (const Blend* blend : system.blends) {
    if (!blend->enabled) continue;
    blends.emplace_back(toTransform(*blend), blend->weight);
  }
  return ast::Transform(ast::RandomChoiceTransform(blends));
}

stdx::optional<ast::Transform> toCameraTransform(const System& system) {
  if (system.finalBlend && system.finalBlend->enabled) {
    return toTransform(*system.finalBlend);
  }

  return stdx::nullopt;
}

}  // namespace chaoskit::flame
