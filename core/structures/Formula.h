#ifndef CHAOSKIT_CORE_FORMULA_H
#define CHAOSKIT_CORE_FORMULA_H

#include <ast/Formula.h>
#include <ast/WeightedFormula.h>
#include <vector>

namespace chaoskit::core {

struct Formula {
  struct Weight {
    float x;
    float y;
  };

  ast::Formula source;
  std::vector<float> params;
  Weight weight = {1.f, 1.f};
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_FORMULA_H
