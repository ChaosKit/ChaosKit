#include "Blend.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(Blend);

std::ostream &operator<<(std::ostream &stream, const Blend &blend) {
  stream << node_type(blend) << " (" << indent << indentation
         << "pre: " << blend.pre() << indentation << "post: " << blend.post();

  for (const auto &formula : blend.formulas()) {
    stream << indentation << formula;
  }

  return stream << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
