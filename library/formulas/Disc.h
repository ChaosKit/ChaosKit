#ifndef CHAOSKIT_LIBRARY_FORMULAS_DISC_H
#define CHAOSKIT_LIBRARY_FORMULAS_DISC_H

#include <cmath>
#include "library/Formula.h"

namespace chaoskit::library {

class Disc : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());

    return withVariables({
        v("rho") / M_PI * sin(n(M_PI) * "r"),
        v("rho") / M_PI * cos(n(M_PI) * "r"),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_DISC_H
