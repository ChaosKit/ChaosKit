#include "BinaryFunction.h"
#include <magic_enum.hpp>
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(BinaryFunction)

std::ostream &operator<<(std::ostream &stream, const BinaryFunction &function) {
  StreamPrinter printer(stream);
  stream << node_type(function) << " "
         << magic_enum::enum_name<BinaryFunction::Type>(function.type()) << " ("
         << indent << indentation;
  apply_visitor(printer, function.first());
  stream << indentation;
  apply_visitor(printer, function.second());
  return stream << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
