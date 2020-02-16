#include "util.h"
#include <memory>
#include <stdexcept>
#include <string>
#include "ColoringMethod.h"
#include "DeJong.h"
#include "Drain.h"
#include "Linear.h"
#include "coloring_methods/Distance.h"
#include "coloring_methods/Noop.h"
#include "coloring_methods/SingleColor.h"

namespace chaoskit::library {

namespace {

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

std::unique_ptr<ColoringMethod> createColoringMethod(ColoringMethodType type) {
  switch (type._value) {
    case ColoringMethodType::Noop:
      return std::make_unique<coloring_methods::Noop>();
    case ColoringMethodType::SingleColor:
      return std::make_unique<coloring_methods::SingleColor>();
    case ColoringMethodType::Distance:
      return std::make_unique<coloring_methods::Distance>();
    default:
      throw std::runtime_error(std::string("Invalid coloring method type: ") +
                               type._to_string());
  }
}

}  // namespace

ast::Expression source(ColoringMethodType type) {
  return createColoringMethod(type)->source();
}
ast::Formula source(FormulaType type) { return createFormula(type)->source(); }

uint32_t paramCount(ColoringMethodType type) {
  return createColoringMethod(type)->paramCount();
}
uint32_t paramCount(FormulaType type) {
  if (type == +FormulaType::Invalid) {
    return 0;
  }

  return createFormula(type)->paramCount();
}

}  // namespace chaoskit::library
