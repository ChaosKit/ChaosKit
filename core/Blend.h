#ifndef CHAOSKIT_CORE_BLEND_H
#define CHAOSKIT_CORE_BLEND_H

#include <vector>
#include "../ast/Blend.h"
#include "Formula.h"
#include "Transform.h"

namespace chaoskit {
namespace core {

struct Blend {
  Transform pre, post;
  std::vector<Formula> formulas;
  float weight = 1.f;

  ast::Blend toSource() const {
    std::vector<ast::WeightedFormula> weighted_formulas;
    weighted_formulas.reserve(formulas.size());
    std::transform(formulas.begin(), formulas.end(),
                   std::back_inserter(weighted_formulas),
                   std::mem_fn(&Formula::toSource));

    return ast::Blend{std::move(weighted_formulas), pre.toSource(),
                      post.toSource()};
  }
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_BLEND_H
