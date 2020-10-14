#ifndef CHAOSKIT_LIBRARY_FORMULAS_JULIA_H
#define CHAOSKIT_LIBRARY_FORMULAS_JULIA_H

#include <cmath>
#include "library/Formula.h"

namespace chaoskit::library {

class Julia : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("sqrtR") = sqrt(sqrt(input.x() * input.x() + input.y() * input.y()));
    var("rho") = atan2(input.x(), input.y());
    var("omega") = lt(random(), 0.5f) * M_PI;

    return withVariables({
        v("sqrtR") * cos(v("rho") / 2.f + "omega"),
        v("sqrtR") * sin(v("rho") / 2.f + "omega"),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_JULIA_H
