#include "System.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(System);

std::ostream &operator<<(std::ostream &stream, const System &system) {
  stream << node_type(system) << " (" << indent;
  for (const auto &blend : system.blends()) {
    stream << indentation << blend;
  }
  return stream << indentation << "final: " << system.final_blend() << outdent
                << indentation << ")";
}

}  // namespace chaoskit::ast
