#ifndef CHAOSKIT_LIBRARY_FORMULA_H
#define CHAOSKIT_LIBRARY_FORMULA_H

#include "ast/Formula.h"
#include "ast/helpers.h"

namespace chaoskit::library {

class Formula {
 public:
  virtual ~Formula() = default;
  virtual ast::Formula source() = 0;
  virtual uint32_t paramCount() { return 0; }

 protected:
  ast::helpers::InputHelper input;
  ast::helpers::ParameterHelper params;
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULA_H
