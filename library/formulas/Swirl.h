#ifndef CHAOSKIT_LIBRARY_FORMULAS_SWIRL_H
#define CHAOSKIT_LIBRARY_FORMULAS_SWIRL_H

#include "library/Formula.h"

namespace chaoskit::library {

class Swirl : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("r2") = v("r") * "r";

    return withVariables({
        input.x() * sin("r2") - input.y() * cos("r2"),
        input.x() * cos("r2") + input.y() * sin("r2"),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_SWIRL_H
