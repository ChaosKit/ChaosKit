#ifndef CHAOSKIT_LIBRARY_FORMULAS_HEART_H
#define CHAOSKIT_LIBRARY_FORMULAS_HEART_H

#include "library/Formula.h"

namespace chaoskit::library {

class Heart : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());

    return withVariables({
        v("r") * sin(v("rho") * "r"),
        v("r") * -cos(v("rho") * "r"),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_HEART_H
