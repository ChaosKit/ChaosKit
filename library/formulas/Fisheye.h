#ifndef CHAOSKIT_LIBRARY_FORMULAS_FISHEYE_H
#define CHAOSKIT_LIBRARY_FORMULAS_FISHEYE_H

#include "library/Formula.h"

namespace chaoskit::library {

class Fisheye : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    return withVariables({
        n(2.f) / (v("r") + 1.f) * input.y(),
        n(2.f) / (v("r") + 1.f) * input.x(),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_FISHEYE_H
