#ifndef CHAOSKIT_AST_BLEND_H
#define CHAOSKIT_AST_BLEND_H

#include <vector>
#include "Transform.h"
#include "WeightedFormula.h"

namespace chaoskit::ast {

class Blend {
 public:
  Blend() : formulas_(), pre_(), post_() {}
  explicit Blend(std::vector<WeightedFormula> formulas)
      : formulas_(std::move(formulas)), pre_(), post_() {}
  Blend(std::vector<WeightedFormula> formulas, const Transform &pre,
        const Transform &post)
      : formulas_(std::move(formulas)), pre_(pre), post_(post) {}

  [[nodiscard]] const std::vector<WeightedFormula> &formulas() const {
    return formulas_;
  }
  [[nodiscard]] const Transform &pre() const { return pre_; }
  [[nodiscard]] const Transform &post() const { return post_; }

  bool operator==(const Blend &other) const {
    return formulas_ == other.formulas_ && pre_ == other.pre_ &&
           post_ == other.post_;
  }

 private:
  std::vector<WeightedFormula> formulas_;
  Transform pre_, post_;
};

std::ostream &operator<<(std::ostream &stream, const Blend &blend);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_BLEND_H
