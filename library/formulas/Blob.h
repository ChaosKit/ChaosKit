#ifndef CHAOSKIT_LIBRARY_FORMULAS_BLOB_H
#define CHAOSKIT_LIBRARY_FORMULAS_BLOB_H

#include <cmath>
#include "library/Formula.h"

namespace chaoskit::library {

class Blob : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    var("rho") = atan2(input.x(), input.y());
    auto high = params[0];
    auto low = params[1];
    auto waves = params[2];
    var("factor") =
        "r" * (low + (high - low) / 2.f * (sin(waves * "rho") + 1.f));

    return withVariables({
        "factor" * cos("rho"),
        "factor" * sin("rho"),
    });
  }
  uint32_t paramCount() override { return 3; }
  std::vector<float> exampleParams() override { return {1.f, .2f, 4.f}; }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_BLOB_H
