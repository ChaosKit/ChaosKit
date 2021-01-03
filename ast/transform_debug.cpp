#include "transform_debug.h"
#include <magic_enum.hpp>
#include "common_debug.h"
#include "expression_debug.h"

namespace chaoskit::ast {

std::ostream &operator<<(std::ostream &stream,
                         const TransformVariant &transform) {
  transform.match([&stream](const auto &concreteTransform) {
    stream << concreteTransform;
  });
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const Transform &transform) {
  stream << "variant: " << transform.variant() << nl;
  return stream << "color: " << transform.color();
}

std::ostream &operator<<(std::ostream &stream,
                         const AffineTransform &transform) {
  return stream << "Affine Transform";
}

std::ostream &operator<<(std::ostream &stream,
                         const MultiStepTransform &transform) {
  stream << "Multi Step (" << transform.transforms().size() << "):" << indent;

  int i = 0;
  for (const auto &step : transform.transforms()) {
    stream << nl << "[" << i << "]:" << indent << nl << step << outdent;
    ++i;
  }
  return stream << outdent;
}

std::ostream &operator<<(std::ostream &stream,
                         const RandomChoiceTransform &transform) {
  stream << "Random Choice (" << transform.transforms().size()
         << "):" << indent;

  int i = 0;
  for (const auto &[transform, weight] : transform.transforms()) {
    stream << nl << "[" << i << "] weight " << weight << ":" << indent << nl;
    stream << transform << outdent;
    ++i;
  }
  return stream << outdent;
}

std::ostream &operator<<(std::ostream &stream,
                         const WeightedSumTransform &transform) {
  stream << "Weighted Sum (" << transform.transforms().size() << "):" << indent;

  int i = 0;
  for (const auto &[transform, weight] : transform.transforms()) {
    stream << nl << "[" << i << "] weight " << weight << ":" << indent << nl;
    stream << transform << outdent;
    ++i;
  }
  return stream << outdent;
}

}  // namespace chaoskit::ast
