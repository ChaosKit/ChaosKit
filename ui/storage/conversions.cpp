#include "conversions.h"
#include <magic_enum.hpp>
#include "ast/transforms.h"
#include "library/util.h"

namespace chaoskit::ui {

namespace {

std::vector<float> toAffineParams(const Transform& transformProto) {
  return {transformProto.m11(), transformProto.m21(), transformProto.m31(),
          transformProto.m12(), transformProto.m22(), transformProto.m32()};
}

ast::Transform lowerBlend(const Blend& blendProto,
                          const core::TransformIndex& parentIndex,
                          core::TransformParams& params,
                          std::vector<QString>& warnings) {
  std::vector<ast::Transform> steps;
  core::TransformIndex stepIndex = parentIndex.firstChild();

  if (blendProto.has_pre()) {
    steps.emplace_back(ast::AffineTransform{});
    params[stepIndex] = toAffineParams(blendProto.pre());
    stepIndex = stepIndex.nextSibling();
  }

  if (blendProto.formulas_size() > 0) {
    std::vector<ast::Transform> formulaTransforms;
    std::vector<double> formulaWeights;
    std::vector<std::vector<float>> formulaParams;

    for (const auto& formula : blendProto.formulas()) {
      auto formulaType =
          magic_enum::enum_cast<library::FormulaType>(formula.type());
      if (!formulaType) {
        warnings.push_back(QString("Unrecognized formula type: %1")
                               .arg(QString::fromStdString(formula.type())));
        continue;
      }

      formulaTransforms.emplace_back(library::source(formulaType.value()));
      formulaWeights.push_back(formula.weight().x());
      formulaParams.emplace_back(formula.params().begin(),
                                 formula.params().end());
    }

    if (!formulaTransforms.empty()) {
      steps.emplace_back(ast::WeightedSumTransform(std::move(formulaTransforms),
                                                   std::move(formulaWeights)));
      core::TransformIndex formulaIndex = stepIndex.firstChild();
      for (const auto& fp : formulaParams) {
        params[formulaIndex] = fp;
        formulaIndex = formulaIndex.nextSibling();
      }
      stepIndex = stepIndex.nextSibling();
    }
  }

  if (blendProto.has_post()) {
    steps.emplace_back(ast::AffineTransform{});
    params[stepIndex] = toAffineParams(blendProto.post());
    stepIndex = stepIndex.nextSibling();
  }

  // Determine the coloring method
  ast::Expression color = ast::Output(ast::Output::COLOR);
  if (blendProto.has_coloring_method()) {
    const auto& coloringMethod = blendProto.coloring_method();
    switch (coloringMethod.method_case()) {
      case ColoringMethod::kSingleColor:
        color = library::source(library::ColoringMethodType::SingleColor);
        params[parentIndex] = {coloringMethod.single_color()};
        break;
      case ColoringMethod::kDistance:
        color = library::source(library::ColoringMethodType::Distance);
        params[parentIndex] = {coloringMethod.distance()};
        break;
      case ColoringMethod::METHOD_NOT_SET:
        break;
    }
  }

  return ast::Transform(ast::MultiStepTransform(std::move(steps)), color);
}

}  // namespace

core::TransformSystem toTransformSystem(const Blend& blendProto,
                                        std::vector<QString>& warnings) {
  core::TransformParams params;
  core::TransformIndex rootIndex;
  ast::Transform transform =
      lowerBlend(blendProto, rootIndex, params, warnings);
  return {std::move(transform), std::move(params)};
}

core::TransformSystem toTransformSystem(const Blend& blendProto) {
  std::vector<QString> warnings;
  return toTransformSystem(blendProto, warnings);
}

core::CameraSystem toCameraSystem(const System& systemProto,
                                  std::vector<QString>& warnings) {
  bool lifetimeAdjusted = false;
  core::Lifetime normalizedLifetime =
      core::Lifetime::normalize(systemProto.min_lifetime(),
                                systemProto.max_lifetime(), &lifetimeAdjusted);
  if (lifetimeAdjusted) {
    warnings.push_back(
        QString("Particle lifetime [%1, %2] was normalized to [%3, %4]")
            .arg(systemProto.min_lifetime())
            .arg(systemProto.max_lifetime())
            .arg(normalizedLifetime.min())
            .arg(normalizedLifetime.max()));
  }

  stdx::optional<core::TransformSystem> system;
  if (systemProto.has_isolated_blend()) {
    uint32_t isolatedIndex = systemProto.isolated_blend().index();
    if (isolatedIndex < static_cast<uint32_t>(systemProto.blends_size())) {
      system = toTransformSystem(systemProto.blends(isolatedIndex), warnings);
    } else {
      warnings.push_back(QString("Isolated blend index %1 is out of range")
                             .arg(isolatedIndex));
    }
  }

  if (!system) {
    std::vector<ast::Transform> transforms;
    std::vector<double> weights;
    core::TransformParams params;
    core::TransformIndex blendIndex;
    blendIndex = blendIndex.firstChild();

    for (const auto& blend : systemProto.blends()) {
      if (!blend.enabled()) continue;
      transforms.push_back(lowerBlend(blend, blendIndex, params, warnings));
      weights.push_back(blend.weight());
      blendIndex = blendIndex.nextSibling();
    }

    system =
        core::TransformSystem{ast::Transform(ast::RandomChoiceTransform(
                                  std::move(transforms), std::move(weights))),
                              std::move(params)};
  }

  stdx::optional<core::TransformSystem> camera;
  if (systemProto.has_camera() && systemProto.camera().enabled()) {
    camera = toTransformSystem(systemProto.camera(), warnings);
  }

  return core::CameraSystem{std::move(*system), std::move(camera),
                            normalizedLifetime};
}

core::CameraSystem toCameraSystem(const System& systemProto) {
  std::vector<QString> warnings;
  return toCameraSystem(systemProto, warnings);
}

}  // namespace chaoskit::ui
