#ifndef CHAOSKIT_LIBRARY_FORMULAS_HYPERBOLIC_H
#define CHAOSKIT_LIBRARY_FORMULAS_HYPERBOLIC_H

#include "library/Formula.h"

namespace chaoskit::library {

class Hyperbolic : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());

    return withVariables({
        sin("rho") / "r",
        v("r") * cos("rho"),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_HYPERBOLIC_H
