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

  std::array<float, 6> examplePreTransform() override {
    return {0.84660766961652,  0, -0.5000978933354223, 0, 0.84660766961652,
            0.2566045370297865};
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_JULIA_H
