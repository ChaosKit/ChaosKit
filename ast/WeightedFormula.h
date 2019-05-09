#ifndef CHAOSKIT_AST_WEIGHTEDFORMULA_H
#define CHAOSKIT_AST_WEIGHTEDFORMULA_H

#include <ostream>
#include "Formula.h"

namespace chaoskit::ast {

class WeightedFormula {
 public:
  WeightedFormula(const Formula& formula)
      : formula_(formula), weight_x_(1.f), weight_y_(1.f) {}
  WeightedFormula(const Formula& formula, float weight)
      : formula_(formula), weight_x_(weight), weight_y_(weight) {}
  WeightedFormula(const Formula& formula, float weight_x, float weight_y)
      : formula_(formula), weight_x_(weight_x), weight_y_(weight_y) {}

  const Formula& formula() const { return formula_; }
  float weight_x() const { return weight_x_; }
  float weight_y() const { return weight_y_; }

  bool operator==(const WeightedFormula& other) const {
    return formula_ == other.formula_ && weight_x_ == other.weight_x_ &&
           weight_y_ == other.weight_y_;
  }

 private:
  Formula formula_;
  float weight_x_;
  float weight_y_;
};

std::ostream& operator<<(std::ostream& stream, const WeightedFormula& formula);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_WEIGHTEDFORMULA_H
