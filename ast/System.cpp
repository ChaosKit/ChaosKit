#include "System.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(System);

std::ostream &operator<<(std::ostream &stream, const System &system) {
  stream << node_type(system) << "(";
  for (const auto &blend : system.blends()) {
    stream << blend << ", ";
  }
  return stream << system.final_blend() << ")";
}

}  // namespace ast
}  // namespace chaoskit
