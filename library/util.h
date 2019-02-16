#ifndef CHAOSKIT_LIBRARY_UTIL_H
#define CHAOSKIT_LIBRARY_UTIL_H

#include "Formula.h"
#include "FormulaType.h"

namespace chaoskit {
namespace library {

std::unique_ptr<Formula> createFormula(FormulaType type);
uint32_t paramCount(FormulaType type);

}  // namespace library
}  // namespace chaoskit

#endif  // CHAOSKIT_LIBRARY_UTIL_H
