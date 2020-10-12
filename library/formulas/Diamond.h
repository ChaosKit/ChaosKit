#ifndef CHAOSKIT_LIBRARY_FORMULAS_DIAMOND_H
#define CHAOSKIT_LIBRARY_FORMULAS_DIAMOND_H

#include "library/Formula.h"

namespace chaoskit::library {

class Diamond : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());

    return withVariables({
        sin("rho") * cos("r"),
        cos("rho") * sin("r"),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_DIAMOND_H
