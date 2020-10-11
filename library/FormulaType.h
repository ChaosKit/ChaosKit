#ifndef CHAOSKIT_LIBRARY_FORMULATYPE_H
#define CHAOSKIT_LIBRARY_FORMULATYPE_H

namespace chaoskit {
namespace library {

enum class FormulaType {
  Invalid = 0,
  DeJong,
  Drain,
  Linear,
  Quadratic,
  Spherical,
  Tinkerbell,
  Trigonometric,
};

}  // namespace library
}  // namespace chaoskit

#endif  // CHAOSKIT_LIBRARY_FORMULATYPE_H
