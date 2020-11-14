#ifndef CHAOSKIT_FLAME_FORMULA_H
#define CHAOSKIT_FLAME_FORMULA_H

#include <vector>
#include "ast/Formula.h"
#include "library/FormulaType.h"
#include "library/util.h"

namespace chaoskit::flame {

struct Formula {
  struct Weight {
    float x;
    float y;
  };

  ast::Formula source;
  library::FormulaType type = library::FormulaType::Invalid;
  std::vector<float> params = {};
  Weight weight = {1.f, 1.f};

  void setType(library::FormulaType newType) {
    type = newType;
    source = library::source(newType);
  }
};

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_FORMULA_H
