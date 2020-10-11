#ifndef CHAOSKIT_LIBRARY_FORMULAS_SPHERICAL_H
#define CHAOSKIT_LIBRARY_FORMULAS_SPHERICAL_H

#include "library/Formula.h"

namespace chaoskit::library {

class Spherical : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    var("r") = sqrt(input.x() * input.x() + input.y() * input.y());
    return withVariables({
        input.x() / (ast::VariableName("r") * ast::VariableName("r")),
        input.y() / (ast::VariableName("r") * ast::VariableName("r")),
    });
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_SPHERICAL_H
