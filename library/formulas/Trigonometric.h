#ifndef CHAOSKIT_LIBRARY_FORMULAS_TRIGONOMETRIC_H
#define CHAOSKIT_LIBRARY_FORMULAS_TRIGONOMETRIC_H

#include "library/Formula.h"

namespace chaoskit::library {

class Trigonometric : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;

    return {plus(params[0] * sin(params[1] * input.y()),
                 params[2] * cos(params[3] * input.x())),
            plus(params[4] * sin(params[5] * input.x()),
                 params[6] * cos(params[7] * input.y()))};
  }

  uint32_t paramCount() override { return 8; }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_TRIGONOMETRIC_H
