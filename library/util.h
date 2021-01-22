#ifndef CHAOSKIT_LIBRARY_UTIL_H
#define CHAOSKIT_LIBRARY_UTIL_H

#include <memory>
#include <vector>
#include "ColoringMethod.h"
#include "ColoringMethodType.h"
#include "Formula.h"
#include "FormulaType.h"
#include "ast/Expression.h"
#include "ast/Formula.h"

namespace chaoskit::library {

std::unique_ptr<Formula> createFormula(FormulaType type);
std::unique_ptr<ColoringMethod> createColoringMethod(ColoringMethodType type);

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

std::vector<float> exampleParams(FormulaType type);
template <class T>
std::vector<float> exampleParams() {
  return T().exampleParams();
}

std::array<float, 6> examplePreTransform(FormulaType type);
template <typename T>
std::array<float, 6> examplePreTransform() {
  return T().examplePreTransform();
}

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_UTIL_H
