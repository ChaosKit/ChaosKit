#ifndef CHAOSKIT_LIBRARY_COLORING_METHODS_NOOP_H
#define CHAOSKIT_LIBRARY_COLORING_METHODS_NOOP_H

#include "library/ColoringMethod.h"

namespace chaoskit::library::coloring_methods {

class Noop : public ColoringMethod {
 public:
  ast::Expression source() override { return output.color(); }
};

}  // namespace chaoskit::library::coloring_methods

#endif  // CHAOSKIT_LIBRARY_COLORING_METHODS_NOOP_H
