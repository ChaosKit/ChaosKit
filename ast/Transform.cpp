#include "types.h"
#include "Transform.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(Transform);

std::ostream &operator<<(std::ostream &stream, const Transform &transform) {
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

}
}
