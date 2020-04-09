#include "UnaryFunction.h"
#include <magic_enum.hpp>
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(UnaryFunction)

std::ostream &operator<<(std::ostream &stream, const UnaryFunction &function) {
  StreamPrinter printer(stream);
  stream << node_type(function) << " "
         << magic_enum::enum_name<UnaryFunction::Type>(function.type()) << " ("
         << indent << indentation;
  apply_visitor(printer, function.argument());
  return stream << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
