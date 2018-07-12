#ifndef CHAOSKIT_CORE_FORMULA_H
#define CHAOSKIT_CORE_FORMULA_H

#include <ast/Formula.h>
#include <ast/WeightedFormula.h>
#include <vector>

namespace chaoskit {
namespace core {

struct Formula {
  ast::Formula source;
  std::vector<float> params;
  float weight_x = 1.f;
  float weight_y = 1.f;

  ast::WeightedFormula toSource() const {
    return ast::WeightedFormula{source, weight_x, weight_y};
  }

  template <class Type>
  static Formula build(std::vector<float> params = {}) {
    return Formula{Type().source(), std::move(params)};
  }
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_FORMULA_H
