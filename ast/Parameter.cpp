#include "Parameter.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(Parameter)

std::ostream &operator<<(std::ostream &stream, const Parameter &parameter) {
  return stream << node_type(parameter) << "(" << parameter.index() << ")";
}

}  // namespace ast
}  // namespace chaoskit
