#ifndef CHAOSKIT_LIBRARY_FORMULAS_TINKERBELL_H
#define CHAOSKIT_LIBRARY_FORMULAS_TINKERBELL_H

#include "library/Formula.h"

namespace chaoskit::library {

class Tinkerbell : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    return {
        input.x() * input.x() - input.y() * input.y() + params[0] * input.x() +
            params[1] * input.y(),
        2.f * input.x() * input.y() + params[2] * input.x() +
            params[3] * input.y(),
    };
  }

  uint32_t paramCount() override { return 4; }

  std::vector<float> exampleParams() override {
    return {0.8633418083190918, -1.6312825679779053, 0.35952901840209961,
            -1.1720921993255615};
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_TINKERBELL_H
