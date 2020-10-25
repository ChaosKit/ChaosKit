#include "debug.h"
#include <magic_enum.hpp>

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

// Expressions

std::ostream &operator<<(std::ostream &stream, const Expression &expression) {
  expression.match([&stream](const auto &concreteExpression) {
    stream << concreteExpression;
  });
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const BinaryFunction &function) {
  stream << "Binary Function "
         << magic_enum::enum_name<BinaryFunction::Type>(function.type()) << ":"
         << indent << nl;
  stream << "[0] " << function.first() << nl;
  stream << "[1] " << function.second();
  return stream << outdent;
}

std::ostream &operator<<(std::ostream &stream, const Input &input) {
  return stream << "Input " << magic_enum::enum_name<Input::Type>(input.type());
}

std::ostream &operator<<(std::ostream &stream, const Output &output) {
  return stream << "Output "
                << magic_enum::enum_name<Output::Type>(output.type());
}

std::ostream &operator<<(std::ostream &stream, const Parameter &parameter) {
  return stream << "Parameter " << parameter.index();
}

std::ostream &operator<<(std::ostream &stream, const RandomNumber &) {
  return stream << "Random Number";
}

std::ostream &operator<<(std::ostream &stream, const UnaryFunction &function) {
  stream << "Unary Function "
         << magic_enum::enum_name<UnaryFunction::Type>(function.type()) << ":"
         << indent << nl;
  return stream << function.argument() << outdent;
}

std::ostream &operator<<(std::ostream &stream,
                         const VariableDeclaration &declaration) {
  stream << "Variable " << declaration.name() << " =" << indent << nl;
  return stream << declaration.definition() << outdent;
}

std::ostream &operator<<(std::ostream &stream, const VariableName &name) {
  return stream << "Variable " << name.name();
}

// Transforms

std::ostream &operator<<(std::ostream &stream,
                         const PositionTransform &transform) {
  transform.match([&stream](const auto &concreteTransform) {
    stream << concreteTransform;
  });
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const Transform &transform) {
  stream << "position: " << transform.position() << nl;
  stream << "color: " << transform.color() << nl;
}

std::ostream &operator<<(std::ostream &stream,
                         const AffineTransform &transform) {
  return stream << "Affine Transform";
}

std::ostream &operator<<(std::ostream &stream, const Formula &formula) {
  stream << "Formula:" << indent << nl;

  stream << "variables: ";
  if (formula.variables().empty()) {
    stream << "none";
  } else {
    stream << indent;
    for (const auto &var : formula.variables()) {
      stream << nl << var;
    }
    stream << outdent;
  }

  stream << nl << "x: " << formula.x();
  stream << nl << "y: " << formula.y();
  return stream << outdent;
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
