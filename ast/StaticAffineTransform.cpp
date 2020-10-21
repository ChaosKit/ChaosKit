#include "StaticAffineTransform.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(StaticAffineTransform);

std::ostream &operator<<(std::ostream &stream,
                         const StaticAffineTransform &transform) {
  stream << node_type(transform) << "(";

  bool prepend_comma = false;
  for (const auto &param : transform.params()) {
    if (prepend_comma) {
      stream << ", ";
    }
    stream << param;
    prepend_comma = true;
  }

  return stream << ")";
}

}  // namespace chaoskit::ast
