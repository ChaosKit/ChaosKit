#ifndef CHAOSKIT_LIBRARY_COLORINGMETHOD_H
#define CHAOSKIT_LIBRARY_COLORINGMETHOD_H

#include "ast/Expression.h"
#include "ast/helpers.h"

namespace chaoskit::library {

class ColoringMethod {
 public:
  virtual ~ColoringMethod() = default;
  virtual ast::Expression source() = 0;
  virtual uint32_t paramCount() { return 0; }

 protected:
  ast::helpers::InputHelper input;
  ast::helpers::OutputHelper output;
  ast::helpers::ParameterHelper params;
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_COLORINGMETHOD_H
