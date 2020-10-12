#ifndef CHAOSKIT_LIBRARY_FORMULAS_SPIRAL_H
#define CHAOSKIT_LIBRARY_FORMULAS_SPIRAL_H

#include "library/Formula.h"

namespace chaoskit::library {

class Spiral : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());

    return withVariables({
        cos("rho") + sin("r") / "r",
        sin("rho") - cos("r") / "r",
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_SPIRAL_H
