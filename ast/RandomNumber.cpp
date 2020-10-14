#include "RandomNumber.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(RandomNumber)

std::ostream &operator<<(std::ostream &stream, const RandomNumber &random) {
  return stream << node_type(random);
}

}  // namespace chaoskit::ast
