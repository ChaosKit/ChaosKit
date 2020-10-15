#ifndef CHAOSKIT_LIBRARY_FORMULAS_BENT_H
#define CHAOSKIT_LIBRARY_FORMULAS_BENT_H

#include "library/Formula.h"

namespace chaoskit::library {

class Bent : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    return {input.x() * (2.f - gte(input.x(), 0.f)),
            input.y() / (2.f - gte(input.y(), 0.f))};
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_BENT_H
