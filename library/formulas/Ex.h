#ifndef CHAOSKIT_LIBRARY_FORMULAS_EX_H
#define CHAOSKIT_LIBRARY_FORMULAS_EX_H

#include "library/Formula.h"

namespace chaoskit::library {

class Ex : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());
    var("p0") = sin(v("rho") + "r");
    var("p1") = sin(v("rho") - "r");

    return withVariables({
        v("r") * (pow("p0", 3.f) + pow("p1", 3.f)),
        v("r") * (pow("p0", 3.f) - pow("p1", 3.f)),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_EX_H
