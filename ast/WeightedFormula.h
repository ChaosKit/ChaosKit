#ifndef CHAOSKIT_AST_WEIGHTEDFORMULA_H
#define CHAOSKIT_AST_WEIGHTEDFORMULA_H

#include <ostream>
#include <utility>
#include "Formula.h"

namespace chaoskit::ast {

class WeightedFormula {
 public:
  explicit WeightedFormula(Formula formula)
      : formula_(std::move(formula)), weight_x_(1.f), weight_y_(1.f) {}
  WeightedFormula(Formula formula, float weight)
      : formula_(std::move(formula)), weight_x_(weight), weight_y_(weight) {}
  WeightedFormula(Formula formula, float weight_x, float weight_y)
      : formula_(std::move(formula)),
        weight_x_(weight_x),
        weight_y_(weight_y) {}

  [[nodiscard]] const Formula& formula() const { return formula_; }
  [[nodiscard]] float weight_x() const { return weight_x_; }
  [[nodiscard]] float weight_y() const { return weight_y_; }

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
