#ifndef CHAOSKIT_LIBRARY_FORMULAS_EXPONENTIAL_H
#define CHAOSKIT_LIBRARY_FORMULAS_EXPONENTIAL_H

#include <cmath>
#include "library/Formula.h"

namespace chaoskit::library {

class Exponential : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    return {
        exp(input.x() - 1.f) * cos(n(M_PI) * input.y()),
        exp(input.x() - 1.f) * sin(n(M_PI) * input.y()),
    };
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_EXPONENTIAL_H
