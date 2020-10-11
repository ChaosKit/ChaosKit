#include "VariableName.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(VariableName)

std::ostream& operator<<(std::ostream& stream, const VariableName& name) {
  return stream << node_type(name) << "(" << name.name() << ")";
}

}  // namespace chaoskit::ast
