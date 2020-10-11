#ifndef CHAOSKIT_LIBRARY_FORMULAS_SINUSOIDAL_H
#define CHAOSKIT_LIBRARY_FORMULAS_SINUSOIDAL_H

#include "library/Formula.h"

namespace chaoskit::library {

class Sinusoidal : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    return {sin(input.x()), sin(input.y())};
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_SINUSOIDAL_H
