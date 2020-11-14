#include "toAst.h"

#include <vector>

#include "ast/transforms.h"

namespace chaoskit::flame {

namespace {

ast::Transform getTransformForBlends(const System& system) {
  if (system.isolatedBlend) {
    return toAst(*system.isolatedBlend);
  }

  std::vector<ast::TransformWithWeight> blends;
  for (const Blend* blend : system.blends) {
    if (!blend->enabled) continue;
    blends.emplace_back(toAst(*blend), blend->weight);
  }
  return ast::Transform(ast::RandomChoiceTransform(blends));
}

}  // namespace

ast::Transform toAst(const BlendBase& blend) {
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

ast::Transform toAst(const System& system) {
  auto blendTransform = getTransformForBlends(system);

  if (system.finalBlend && system.finalBlend->enabled) {
    return ast::Transform(ast::MultiStepTransform{std::move(blendTransform),
                                                  toAst(*system.finalBlend)});
  }

  return blendTransform;
}

}  // namespace chaoskit::flame
