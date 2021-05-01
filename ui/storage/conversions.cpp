#include "conversions.h"
#include <magic_enum.hpp>
#include "ast/transforms.h"
#include "library/util.h"

namespace chaoskit::ui {

namespace {

ast::Transform toTransform(const Blend& blendProto) {
  std::vector<ast::Transform> steps;
  if (blendProto.has_pre()) {
    steps.emplace_back(ast::AffineTransform{});
  }

  if (blendProto.formulas_size() > 0) {
    std::vector<ast::Transform> transforms;
    std::vector<double> weights;

    // Translate formulas
    for (const auto& formula : blendProto.formulas()) {
      auto formulaType =
          magic_enum::enum_cast<library::FormulaType>(formula.type());
      transforms.emplace_back(library::source(formulaType.value()));
      weights.push_back(formula.weight().x());
    }

    steps.emplace_back(
        ast::WeightedSumTransform(std::move(transforms), std::move(weights)));
  }

  if (blendProto.has_post()) {
    steps.emplace_back(ast::AffineTransform{});
  }

  // Determine the coloring method
  ast::Expression color = ast::Output(ast::Output::COLOR);
  if (blendProto.has_coloring_method()) {
    switch (blendProto.coloring_method().method_case()) {
      case ColoringMethod::kSingleColor:
        color = library::source(library::ColoringMethodType::SingleColor);
        break;
      case ColoringMethod::kDistance:
        color = library::source(library::ColoringMethodType::Distance);
        break;
      case ColoringMethod::METHOD_NOT_SET:
        break;
    }
  }

  return ast::Transform(ast::MultiStepTransform(std::move(steps)), color);
}

void populateTransformParams(std::vector<float>& params,
                             const Transform& transformProto) {
  params = {transformProto.m11(), transformProto.m21(), transformProto.m31(),
            transformProto.m12(), transformProto.m22(), transformProto.m32()};
}

void populateParams(core::TransformParams& params, const Blend& blendProto,
                    core::TransformIndex parentIndex = core::TransformIndex()) {
  auto index = parentIndex.firstChild();

  if (blendProto.has_pre()) {
    populateTransformParams(params[index], blendProto.pre());
    index = index.nextSibling();
  }

  if (blendProto.formulas_size() > 0) {
    index = index.firstChild();
    for (const auto& formula : blendProto.formulas()) {
      params[index] =
          std::vector<float>(formula.params().begin(), formula.params().end());
      index = index.nextSibling();
    }
    index = index.parent().nextSibling();
  }

  if (blendProto.has_post()) {
    populateTransformParams(params[index], blendProto.post());
  }

  // Set params for the coloring method on the parent.
  if (blendProto.has_coloring_method()) {
    const auto& coloringMethod = blendProto.coloring_method();
    switch (coloringMethod.method_case()) {
      case ColoringMethod::kSingleColor:
        params[parentIndex] = {coloringMethod.single_color()};
        break;
      case ColoringMethod::kDistance:
        params[parentIndex] = {coloringMethod.distance()};
        break;
      case ColoringMethod::METHOD_NOT_SET:
        break;
    }
  }
}

ast::Transform toTransform(const System& systemProto) {
  if (systemProto.has_isolated_blend()) {
    return toTransform(
        systemProto.blends(systemProto.isolated_blend().index()));
  }

  std::vector<ast::Transform> transforms;
  std::vector<double> weights;
  for (const auto& blend : systemProto.blends()) {
    if (!blend.enabled()) continue;
    transforms.push_back(std::move(toTransform(blend)));
    weights.push_back(blend.weight());
  }
  return ast::Transform(
      ast::RandomChoiceTransform(std::move(transforms), std::move(weights)));
}

core::TransformParams toParams(const Blend& blendProto) {
  core::TransformParams params;
  populateParams(params, blendProto);
  return params;
}

core::TransformParams toParams(const System& systemProto) {
  core::TransformParams params;

  if (systemProto.has_isolated_blend()) {
    populateParams(params,
                   systemProto.blends(systemProto.isolated_blend().index()));
    return params;
  }

  core::TransformIndex index;
  index = index.firstChild();  // Random Choice

  for (const auto& blend : systemProto.blends()) {
    if (!blend.enabled()) continue;
    populateParams(params, blend, index);
    index = index.nextSibling();
  }

  return params;
}

}  // namespace

core::CameraSystem toCameraSystem(const System& systemProto) {
  std::optional<core::TransformSystem> camera;
  if (systemProto.has_final_blend() && systemProto.final_blend().enabled()) {
    camera = core::TransformSystem{toTransform(systemProto.final_blend()),
                                   toParams(systemProto.final_blend())};
  }
  return {{toTransform(systemProto), toParams(systemProto)}, std::move(camera)};
}

}  // namespace chaoskit::ui
