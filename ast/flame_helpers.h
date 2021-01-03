#ifndef CHAOSKIT_AST_FLAME_HELPER_H
#define CHAOSKIT_AST_FLAME_HELPER_H

#include "System.h"

namespace chaoskit::ast::helpers {

static System make_system(const Formula& f) {
  return System{{LimitedBlend(Blend{{WeightedFormula(f)}}, 1.f)}};
}

}  // namespace chaoskit::ast::helpers

#endif  // CHAOSKIT_AST_FLAME_HELPER_H
