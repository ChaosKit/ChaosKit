#include "util.h"
#include <magic_enum.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include "ColoringMethod.h"
#include "coloring_methods/Distance.h"
#include "coloring_methods/Noop.h"
#include "coloring_methods/SingleColor.h"
#include "formulas/DeJong.h"
#include "formulas/Drain.h"
#include "formulas/Linear.h"
#include "formulas/Trigonometric.h"

namespace chaoskit::library {

namespace {

std::unique_ptr<Formula> createFormula(FormulaType type) {
  switch (type) {
    case FormulaType::DeJong:
      return std::make_unique<DeJong>();
    case FormulaType::Drain:
      return std::make_unique<Drain>();
    case FormulaType::Linear:
      return std::make_unique<Linear>();
    case FormulaType::Trigonometric:
      return std::make_unique<Trigonometric>();
    case FormulaType::Invalid:
    default:
      throw std::runtime_error(
          std::string("Invalid formula type: ") +
          std::string(magic_enum::enum_name<FormulaType>(type)));
  }
}

std::unique_ptr<ColoringMethod> createColoringMethod(ColoringMethodType type) {
  switch (type) {
    case ColoringMethodType::Noop:
      return std::make_unique<coloring_methods::Noop>();
    case ColoringMethodType::SingleColor:
      return std::make_unique<coloring_methods::SingleColor>();
    case ColoringMethodType::Distance:
      return std::make_unique<coloring_methods::Distance>();
    default:
      throw std::runtime_error(
          std::string("Invalid coloring method type: ") +
          std::string(magic_enum::enum_name<ColoringMethodType>(type)));
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
  if (type == FormulaType::Invalid) {
    return 0;
  }

  return createFormula(type)->paramCount();
}

}  // namespace chaoskit::library
