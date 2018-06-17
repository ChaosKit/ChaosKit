#ifndef CHAOSKIT_CORE_FORMULA_H
#define CHAOSKIT_CORE_FORMULA_H

#include <vector>
#include "../ast/Formula.h"
#include "../ast/WeightedFormula.h"

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
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_FORMULA_H
