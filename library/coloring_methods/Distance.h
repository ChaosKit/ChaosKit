#ifndef CHAOSKIT_LIBRARY_COLORING_METHODS_DISTANCE_H
#define CHAOSKIT_LIBRARY_COLORING_METHODS_DISTANCE_H

#include "library/ColoringMethod.h"

namespace chaoskit::library::coloring_methods {

class Distance : public ColoringMethod {
 public:
  ast::Expression source() override {
    using namespace ast::helpers;
    auto dx = input.x() - output.x();
    auto dy = input.y() - output.y();
    auto distance = sqrt(plus(dx * dx, dy * dy));

    return frac(distance * params[0]);
  }
  uint32_t paramCount() override { return 1; }
};

}  // namespace chaoskit::library::coloring_methods

#endif  // CHAOSKIT_LIBRARY_COLORING_METHODS_DISTANCE_H
