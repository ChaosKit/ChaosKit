#ifndef CHAOSKIT_CORE_COLORINGMETHOD_H
#define CHAOSKIT_CORE_COLORINGMETHOD_H

#include <algorithm>
#include <vector>
#include "ast/Expression.h"
#include "ast/Input.h"
#include "library/ColoringMethodType.h"
#include "library/util.h"

namespace chaoskit::core {

struct ColoringMethod {
  ast::Expression source = ast::Input(ast::Input::Type::COLOR);
  library::ColoringMethodType type = library::ColoringMethodType::Noop;
  std::vector<float> params = {};

  void setType(library::ColoringMethodType newType) {
    type = newType;
    source = library::source(newType);

    // Fill the params with decent default values
    params.resize(library::paramCount(newType));
    std::fill(params.begin(), params.end(), .4f);
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_STRUCTURES_COLORINGMETHOD_H
