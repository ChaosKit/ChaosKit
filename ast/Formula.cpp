#include "Formula.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(Formula)

std::ostream &operator<<(std::ostream &stream, const Formula &formula) {
  StreamPrinter printer(stream);
  stream << node_type(formula) << " (" << indent << indentation;

  stream << "variables: ";
  if (formula.variables().empty()) {
    stream << "none";
  } else {
    stream << indent;
    for (const auto &var : formula.variables()) {
      stream << indentation << var;
    }
    stream << outdent;
  }

  stream << indentation << "x: ";
  apply_visitor(printer, formula.x());
  stream << indentation << "y: ";
  apply_visitor(printer, formula.y());
  return stream << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
