#ifndef CHAOSKIT_AST_BLEND_H
#define CHAOSKIT_AST_BLEND_H

#include <vector>
#include "Transform.h"
#include "WeightedFormula.h"

namespace chaoskit {
namespace ast {

class Blend {
 public:
  Blend() : formulas_(), pre_(), post_() {}
  Blend(std::initializer_list<WeightedFormula> formulas)
      : formulas_(formulas), pre_(), post_() {}
  Blend(std::initializer_list<WeightedFormula> formulas, const Transform &pre,
        const Transform &post)
      : formulas_(formulas), pre_(pre), post_(post) {}

  const std::vector<WeightedFormula> &formulas() const { return formulas_; }
  const Transform &pre() const { return pre_; }
  const Transform &post() const { return post_; }

  bool operator==(const Blend &other) const {
    return formulas_ == other.formulas_ && pre_ == other.pre_ &&
           post_ == other.post_;
  }

 private:
  std::vector<WeightedFormula> formulas_;
  Transform pre_, post_;
};

std::ostream &operator<<(std::ostream &stream, const Blend &blend);

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_AST_BLEND_H
