#include "expression_debug.h"
#include <magic_enum.hpp>
#include "common_debug.h"

namespace chaoskit::ast {

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

}  // namespace chaoskit::ast
