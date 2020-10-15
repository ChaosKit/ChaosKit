#ifndef CHAOSKIT_LIBRARY_FORMULAS_COSINE_H
#define CHAOSKIT_LIBRARY_FORMULAS_COSINE_H

#include <cmath>
#include "library/Formula.h"

namespace chaoskit::library {

class Cosine : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    return {
        cos(M_PI * input.x()) * cosh(input.y()),
        -sin(M_PI * input.x()) * sinh(input.y()),
    };
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_COSINE_H
