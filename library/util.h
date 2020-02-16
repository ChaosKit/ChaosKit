#ifndef CHAOSKIT_LIBRARY_UTIL_H
#define CHAOSKIT_LIBRARY_UTIL_H

#include "ColoringMethodType.h"
#include "FormulaType.h"
#include "ast/Expression.h"
#include "ast/Formula.h"

namespace chaoskit::library {

ast::Expression source(ColoringMethodType type);
ast::Formula source(FormulaType type);
template <class T>
ast::Formula source() {
  return T().source();
}

uint32_t paramCount(ColoringMethodType type);
uint32_t paramCount(FormulaType type);
template <class T>
uint32_t paramCount() {
  return T().paramCount();
}

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_UTIL_H
