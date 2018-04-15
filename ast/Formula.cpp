#include "types.h"
#include "Formula.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(Formula)

std::ostream &operator<<(std::ostream &stream, const Formula &formula) {
  StreamPrinter printer(stream);
  stream << node_type(formula) << "{";
  apply_visitor(printer, formula.x());
  stream << ", ";
  apply_visitor(printer, formula.y());
  return stream << "}";
}

}  // namespace ast
}  // namespace chaoskit
