#include "BinaryFunction.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(BinaryFunction)

std::ostream &operator<<(std::ostream &stream, const BinaryFunction &function) {
  StreamPrinter printer(stream);
  stream << node_type(function) << ":" << function.type() << "(";
  apply_visitor(printer, function.first());
  stream << ", ";
  apply_visitor(printer, function.second());
  return stream << ")";
}

}  // namespace ast
}  // namespace chaoskit
