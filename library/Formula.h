#ifndef CHAOSKIT_LIBRARY_FORMULA_H
#define CHAOSKIT_LIBRARY_FORMULA_H

#include "ast/Formula.h"
#include "ast/helpers.h"

namespace chaoskit {
namespace library {

class Formula {
 public:
  virtual ast::Formula source() = 0;
 protected:
  ast::helpers::InputHelper input;
  ast::helpers::ParameterHelper params;
};

}
}

#endif //CHAOSKIT_LIBRARY_FORMULA_H
