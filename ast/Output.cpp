#include "Output.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(Output)

std::ostream &operator<<(std::ostream &stream, const Output &output) {
  return stream << node_type(output) << "(" << output.type() << ")";
}

}  // namespace chaoskit::ast
