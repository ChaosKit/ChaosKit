#ifndef CHAOSKIT_LIBRARY_UTIL_H
#define CHAOSKIT_LIBRARY_UTIL_H

#include <ast/Formula.h>
#include "FormulaType.h"

namespace chaoskit {
namespace library {

ast::Formula source(FormulaType type);
template <class T>
ast::Formula source() {
  return T().source();
}

uint32_t paramCount(FormulaType type);
template <class T>
uint32_t paramCount() {
  return T().paramCount();
}

}  // namespace library
}  // namespace chaoskit

#endif  // CHAOSKIT_LIBRARY_UTIL_H
