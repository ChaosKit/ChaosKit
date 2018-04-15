#include "ast/ast.h"
#include "DeJong.h"

namespace chaoskit {
namespace library {

ast::Formula DeJong::source() {
  using namespace ast::helpers;

  return {sin(params[0] * input.y()) - cos(params[1] * input.x()),
          sin(params[2] * input.x()) - cos(params[3] * input.y())};
}

}  // namespace library
}  // namespace chaoskit
