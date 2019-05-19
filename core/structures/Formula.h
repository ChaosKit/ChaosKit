#ifndef CHAOSKIT_CORE_FORMULA_H
#define CHAOSKIT_CORE_FORMULA_H

#include <ast/Formula.h>
#include <ast/WeightedFormula.h>
#include <vector>

namespace chaoskit::core {

struct Formula {
  ast::Formula source;
  std::vector<float> params;
  float weight_x = 1.f;
  float weight_y = 1.f;

  template <class Type>
  static std::shared_ptr<Formula> build(const std::vector<float>& params = {}) {
    auto formula = std::make_shared<Formula>();
    formula->source = Type().source();
    formula->params = params;
    return formula;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_FORMULA_H
