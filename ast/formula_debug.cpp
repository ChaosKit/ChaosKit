#include "formula_debug.h"
#include "common_debug.h"
#include "expression_debug.h"

namespace chaoskit::ast {

std::ostream &operator<<(std::ostream &stream, const Formula &formula) {
  stream << "Formula:" << indent << nl;

  stream << "variables: ";
  if (formula.variables().empty()) {
    stream << "none";
  } else {
    stream << indent;
    for (const auto &var : formula.variables()) {
      stream << nl << var;
    }
    stream << outdent;
  }

  stream << nl << "x: " << formula.x();
  stream << nl << "y: " << formula.y();
  return stream << outdent;
}

}  // namespace chaoskit::ast
