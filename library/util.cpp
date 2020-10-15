#include "util.h"
#include <magic_enum.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include "ColoringMethod.h"
#include "coloring_methods/Distance.h"
#include "coloring_methods/Noop.h"
#include "coloring_methods/SingleColor.h"
#include "formulas/Bent.h"
#include "formulas/Cosine.h"
#include "formulas/DeJong.h"
#include "formulas/Diamond.h"
#include "formulas/Disc.h"
#include "formulas/Drain.h"
#include "formulas/Ex.h"
#include "formulas/Exponential.h"
#include "formulas/Fisheye.h"
#include "formulas/Handkerchief.h"
#include "formulas/Heart.h"
#include "formulas/Horseshoe.h"
#include "formulas/Hyperbolic.h"
#include "formulas/Julia.h"
#include "formulas/Linear.h"
#include "formulas/Polar.h"
#include "formulas/Quadratic.h"
#include "formulas/Sinusoidal.h"
#include "formulas/Spherical.h"
#include "formulas/Spiral.h"
#include "formulas/Swirl.h"
#include "formulas/Tinkerbell.h"
#include "formulas/Trigonometric.h"

namespace chaoskit::library {

std::unique_ptr<Formula> createFormula(FormulaType type) {
  switch (type) {
    case FormulaType::Bent:
      return std::make_unique<Bent>();
    case FormulaType::Cosine:
      return std::make_unique<Cosine>();
    case FormulaType::DeJong:
      return std::make_unique<DeJong>();
    case FormulaType::Diamond:
      return std::make_unique<Diamond>();
    case FormulaType::Disc:
      return std::make_unique<Disc>();
    case FormulaType::Drain:
      return std::make_unique<Drain>();
    case FormulaType::Ex:
      return std::make_unique<Ex>();
    case FormulaType::Exponential:
      return std::make_unique<Exponential>();
    case FormulaType::Fisheye:
      return std::make_unique<Fisheye>();
    case FormulaType::Handkerchief:
      return std::make_unique<Handkerchief>();
    case FormulaType::Heart:
      return std::make_unique<Heart>();
    case FormulaType::Horseshoe:
      return std::make_unique<Horseshoe>();
    case FormulaType::Hyperbolic:
      return std::make_unique<Hyperbolic>();
    case FormulaType::Julia:
      return std::make_unique<Julia>();
    case FormulaType::Linear:
      return std::make_unique<Linear>();
    case FormulaType::Polar:
      return std::make_unique<Polar>();
    case FormulaType::Quadratic:
      return std::make_unique<Quadratic>();
    case FormulaType::Sinusoidal:
      return std::make_unique<Sinusoidal>();
    case FormulaType::Spherical:
      return std::make_unique<Spherical>();
    case FormulaType::Spiral:
      return std::make_unique<Spiral>();
    case FormulaType::Swirl:
      return std::make_unique<Swirl>();
    case FormulaType::Tinkerbell:
      return std::make_unique<Tinkerbell>();
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

std::vector<float> exampleParams(FormulaType type) {
  if (type == FormulaType::Invalid) {
    return {};
  }
  return createFormula(type)->exampleParams();
}

ast::Transform examplePreTransform(FormulaType type) {
  if (type == FormulaType::Invalid) {
    return {};
  }
  return createFormula(type)->examplePreTransform();
}

}  // namespace chaoskit::library
