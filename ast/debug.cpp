#include "debug.h"

namespace chaoskit::ast {

namespace {

int indentLocation = std::ios_base::xalloc();

std::ios_base &indent(std::ios_base &os) {
  os.iword(indentLocation) += 2;
  return os;
}
std::ios_base &outdent(std::ios_base &os) {
  long &indentSize = os.iword(indentLocation);
  indentSize = std::max(indentSize - 2L, 0L);
  return os;
}

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits> &nl(std::basic_ostream<CharT, Traits> &os) {
  os << std::endl
     << std::string(static_cast<size_t>(os.iword(indentLocation)), ' ');
  return os;
}

}  // namespace

std::ostream &operator<<(std::ostream &stream,
                         const PositionTransform &transform) {
  transform.match([&stream](const auto &concreteTransform) {
    stream << concreteTransform;
  });
  return stream;
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
    stream << nl << "[" << i << "]:" << indent << nl << transform << outdent;
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
