#ifndef CHAOSKIT_AST_BLEND_H
#define CHAOSKIT_AST_BLEND_H

#include <vector>
#include "Expression.h"
#include "StaticAffineTransform.h"
#include "WeightedFormula.h"

namespace chaoskit::ast {

class Blend {
 public:
  Blend() : formulas_(), pre_(), post_() {}
  explicit Blend(std::vector<WeightedFormula> formulas)
      : formulas_(std::move(formulas)), pre_(), post_() {}
  Blend(std::vector<WeightedFormula> formulas, const StaticAffineTransform &pre,
        const StaticAffineTransform &post)
      : formulas_(std::move(formulas)), pre_(pre), post_(post) {}
  Blend(std::vector<WeightedFormula> formulas, const StaticAffineTransform &pre,
        const StaticAffineTransform &post, const Expression &coloringMethod)
      : formulas_(std::move(formulas)),
        pre_(pre),
        post_(post),
        coloringMethod_(coloringMethod) {}

  [[nodiscard]] const std::vector<WeightedFormula> &formulas() const {
    return formulas_;
  }
  [[nodiscard]] const StaticAffineTransform &pre() const { return pre_; }
  [[nodiscard]] const StaticAffineTransform &post() const { return post_; }
  [[nodiscard]] const Expression &coloringMethod() const {
    return coloringMethod_;
  }

  bool operator==(const Blend &other) const {
    return formulas_ == other.formulas_ && pre_ == other.pre_ &&
           post_ == other.post_ && coloringMethod_ == other.coloringMethod_;
  }

 private:
  std::vector<WeightedFormula> formulas_;
  StaticAffineTransform pre_, post_;
  Expression coloringMethod_ = Input(Input::Type::COLOR);
};

std::ostream &operator<<(std::ostream &stream, const Blend &blend);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_BLEND_H
