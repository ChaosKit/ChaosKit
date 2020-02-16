#ifndef CHAOSKIT_LIBRARY_COLORING_METHODS_SINGLECOLOR_H
#define CHAOSKIT_LIBRARY_COLORING_METHODS_SINGLECOLOR_H

#include "library/ColoringMethod.h"

namespace chaoskit::library::coloring_methods {

class SingleColor : public ColoringMethod {
 public:
  ast::Expression source() override {
    using namespace ast::helpers;
    return plus(input.color(), params[0]) / 2.f;
  }
  uint32_t paramCount() override { return 1; }
};

}  // namespace chaoskit::library::coloring_methods

#endif  // CHAOSKIT_LIBRARY_COLORING_METHODS_SINGLECOLOR_H
