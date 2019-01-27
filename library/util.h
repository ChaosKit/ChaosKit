#ifndef CHAOSKIT_LIBRARY_UTILS_H
#define CHAOSKIT_LIBRARY_UTILS_H

#include "DeJong.h"
#include "Formula.h"
#include "FormulaType.h"

namespace chaoskit {
namespace library {

std::unique_ptr<Formula> createFormula(FormulaType type) {
  switch (type._value) {
    case FormulaType::DeJong:
      return std::make_unique<DeJong>();
    case FormulaType::Invalid:
    default:
      throw std::runtime_error(std::string("Invalid formula type: ") +
                               type._to_string());
  }
}

}  // namespace library
}  // namespace chaoskit

#endif  // CHAOSKIT_LIBRARY_UTILS_H
