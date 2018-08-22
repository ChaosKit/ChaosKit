#include "UnaryFunction.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(UnaryFunction)

std::ostream &operator<<(std::ostream &stream, const UnaryFunction &function) {
  StreamPrinter printer(stream);
  stream << node_type(function) << ":" << function.type() << "(";
  apply_visitor(printer, function.argument());
  return stream << ")";
}

}  // namespace ast
}  // namespace chaoskit
