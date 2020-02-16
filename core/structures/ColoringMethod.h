#ifndef CHAOSKIT_CORE_COLORINGMETHOD_H
#define CHAOSKIT_CORE_COLORINGMETHOD_H

#include <vector>
#include "ast/Expression.h"
#include "library/ColoringMethodType.h"
#include "library/util.h"

namespace chaoskit::core {

struct ColoringMethod {
  ast::Expression source;
  library::ColoringMethodType type = library::ColoringMethodType::Noop;
  std::vector<float> params = {};

  void setType(library::ColoringMethodType newType) {
    type = newType;
    source = library::source(newType);
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_STRUCTURES_COLORINGMETHOD_H
