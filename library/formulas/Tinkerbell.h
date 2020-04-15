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
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_TINKERBELL_H
