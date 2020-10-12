#ifndef CHAOSKIT_LIBRARY_FORMULAS_HORSESHOE_H
#define CHAOSKIT_LIBRARY_FORMULAS_HORSESHOE_H

#include "library/Formula.h"

namespace chaoskit::library {

class Horseshoe : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());

    return withVariables({
        (input.x() - input.y()) * (input.x() + input.y()) / "r",
        2.f * input.x() * input.y(),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_HORSESHOE_H
