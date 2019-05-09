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
  static Formula build(std::vector<float> params = {}) {
    return Formula{Type().source(), std::move(params)};
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_FORMULA_H
