#ifndef CHAOSKIT_AST_BLEND_H
#define CHAOSKIT_AST_BLEND_H

#include <vector>
#include "WeightedFormula.h"

namespace chaoskit {
namespace ast {

class Blend {
 public:
  Blend(std::initializer_list<WeightedFormula> formulas)
      : formulas_(formulas) {}

  const std::vector<WeightedFormula>& formulas() const { return formulas_; }

  bool operator==(const Blend& other) const {
    return formulas_ == other.formulas_;
  }

 private:
  std::vector<WeightedFormula> formulas_;
};

std::ostream& operator<<(std::ostream& stream, const Blend& blend);

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_AST_BLEND_H
