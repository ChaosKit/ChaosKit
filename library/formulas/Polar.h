#ifndef CHAOSKIT_LIBRARY_FORMULAS_POLAR_H
#define CHAOSKIT_LIBRARY_FORMULAS_POLAR_H

#include <cmath>
#include "library/Formula.h"

namespace chaoskit::library {

class Polar : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());

    return withVariables({v("rho") / M_PI, v("r") - 1.f});
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_POLAR_H
