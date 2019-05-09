#include "LimitedBlend.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(LimitedBlend);

std::ostream &operator<<(std::ostream &stream, const LimitedBlend &blend) {
  return stream << node_type(blend) << " (" << indent << indentation
                << "limit: " << blend.limit() << indentation << blend.blend()
                << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
