#include "types.h"
#include "Blend.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(Blend);

std::ostream &operator<<(std::ostream &stream, const Blend &blend) {
  stream << node_type(blend) << "[";

  bool prepend_comma = false;
  for (const auto &formula : blend.formulas()) {
    if (prepend_comma) {
      stream << ", ";
    }
    stream << formula;
    prepend_comma = true;
  }

  return stream << "]";
}

}
}
