#include "toSource.h"
#include <library/util.h>
#include <QDebug>

namespace chaoskit {
namespace ui {

ast::Blend toSource(const Blend *blend) {
  if (blend == nullptr) {
    return ast::Blend();
  }

  std::vector<ast::WeightedFormula> weighted_formulas;
  weighted_formulas.reserve(static_cast<size_t>(blend->formulaCount()));
  for (const auto &formula : blend->formulas()) {
    weighted_formulas.push_back(toSource(formula));
  }

  return ast::Blend{std::move(weighted_formulas), toSource(blend->pre()),
                    toSource(blend->post())};
}

ast::WeightedFormula toSource(const Formula *formula) {
  return ast::WeightedFormula{library::createFormula(formula->type())->source(),
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

}  // namespace ui
}  // namespace chaoskit
