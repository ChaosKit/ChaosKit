#include "LimitedBlend.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(LimitedBlend);

std::ostream &operator<<(std::ostream &stream, const LimitedBlend &blend) {
  return stream << node_type(blend) << "(" << blend.blend() << ", "
                << blend.limit() << ")";
}

}  // namespace ast
}  // namespace chaoskit
