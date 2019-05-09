#include "util.h"
#include <algorithm>
#include <string>

namespace chaoskit::ast {

int StreamPrinter::indentLocation = std::ios_base::xalloc();

std::ios_base &indent(std::ios_base &os) {
  os.iword(StreamPrinter::indentLocation) += 2;
  return os;
}

std::ios_base &outdent(std::ios_base &os) {
  long &indentSize = os.iword(StreamPrinter::indentLocation);
  indentSize = std::max(indentSize - 2L, 0L);
  return os;
}

}  // namespace chaoskit::ast
