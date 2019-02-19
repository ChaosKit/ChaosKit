#include "util.h"
#include <memory>
#include <stdexcept>
#include <string>
#include "DeJong.h"
#include "Drain.h"
#include "Linear.h"

namespace chaoskit {
namespace library {

std::unique_ptr<Formula> createFormula(FormulaType type) {
  switch (type._value) {
    case FormulaType::DeJong:
      return std::make_unique<DeJong>();
    case FormulaType::Drain:
      return std::make_unique<Drain>();
    case FormulaType::Linear:
      return std::make_unique<Linear>();
    case FormulaType::Invalid:
    default:
      throw std::runtime_error(std::string("Invalid formula type: ") +
                               type._to_string());
  }
}

uint32_t paramCount(FormulaType type) {
  if (type == +FormulaType::Invalid) {
    return 0;
  }

  return createFormula(type)->paramCount();
}

}  // namespace library
}  // namespace chaoskit
