#include "Blend.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(Blend);

std::ostream &operator<<(std::ostream &stream, const Blend &blend) {
  stream << node_type(blend) << "(" << blend.pre();

  for (const auto &formula : blend.formulas()) {
    stream << ", " << formula;
  }

  return stream << ", " << blend.post() << ")";
}

}  // namespace ast
}  // namespace chaoskit
