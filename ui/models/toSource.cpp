#include "toSource.h"
#include <library/Linear.h>
#include <library/util.h>
#include <QDebug>
#include <algorithm>

using chaoskit::library::Linear;

namespace chaoskit::ui {

ast::Blend toSource(const Blend *blend) {
  if (blend == nullptr) {
    return ast::Blend();
  }

  std::vector<ast::WeightedFormula> weighted_formulas;

  if (blend->formulaCount() == 1) {
    // Modifying weight of one formula also causes scaling the image. This is
    // technically correct, but undesirable.
    // This mitigates that by balancing it with a linear formula, but only if
    // the weights are smaller than 1.
    Formula *formula = blend->formulaAt(0);
    weighted_formulas.push_back(toSource(formula));

    float linearWeightX = std::max(0.f, 1.f - formula->weightX());
    float linearWeightY = std::max(0.f, 1.f - formula->weightY());
    if (linearWeightX > 0.f || linearWeightY > 0.f) {
      weighted_formulas.emplace_back(library::source<Linear>(), linearWeightX,
                                     linearWeightY);
    }
  } else {
    weighted_formulas.reserve(static_cast<size_t>(blend->formulaCount()));
    for (const Formula *formula : blend->formulas()) {
      weighted_formulas.push_back(toSource(formula));
    }
  }

  return ast::Blend{std::move(weighted_formulas), toSource(blend->pre()),
                    toSource(blend->post())};
}

ast::WeightedFormula toSource(const Formula *formula) {
  return ast::WeightedFormula{library::source(formula->type()),
                              formula->weightX(), formula->weightY()};
}

ast::Transform toSource(const QTransform &transform) {
  return ast::Transform{
      static_cast<float>(transform.m11()), static_cast<float>(transform.m21()),
      static_cast<float>(transform.m31()), static_cast<float>(transform.m12()),
      static_cast<float>(transform.m22()), static_cast<float>(transform.m32())};
}

ast::System toSource(const System *system) {
  // Generate a monotonically increasing vector of limits
  std::vector<float> limits(static_cast<size_t>(system->blendCount()));
  std::transform(system->blends().begin(), system->blends().end(),
                 limits.begin(), std::mem_fn(&Blend::weight));
  std::partial_sum(limits.begin(), limits.end(), limits.begin());

  // Combine blends and limits into LimitedBlends
  std::vector<ast::LimitedBlend> limitedBlends;
  limitedBlends.reserve(static_cast<size_t>(system->blendCount()));
  std::transform(system->blends().begin(), system->blends().end(),
                 limits.begin(), std::back_inserter(limitedBlends),
                 [](const Blend *blend, float limit) {
                   return ast::LimitedBlend{toSource(blend), limit};
                 });

  return ast::System{std::move(limitedBlends), toSource(system->finalBlend())};
}

}  // namespace chaoskit::ui
