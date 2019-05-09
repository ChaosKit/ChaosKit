#include "UnaryFunction.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(UnaryFunction)

std::ostream &operator<<(std::ostream &stream, const UnaryFunction &function) {
  StreamPrinter printer(stream);
  stream << node_type(function) << " " << function.type() << " (" << indent
         << indentation;
  apply_visitor(printer, function.argument());
  return stream << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
