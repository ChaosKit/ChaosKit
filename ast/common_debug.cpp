#include "common_debug.h"

namespace chaoskit::ast {

static int indentLocation = std::ios_base::xalloc();

std::ios_base &indent(std::ios_base &os) {
  os.iword(indentLocation) += 2;
  return os;
}
std::ios_base &outdent(std::ios_base &os) {
  long &indentSize = os.iword(indentLocation);
  indentSize = std::max(indentSize - 2L, 0L);
  return os;
}

std::ostream &nl(std::ostream &os) {
  os << std::endl
     << std::string(static_cast<size_t>(os.iword(indentLocation)), ' ');
  return os;
}

}  // namespace chaoskit::ast
