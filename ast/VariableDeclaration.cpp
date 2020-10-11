#include "VariableDeclaration.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(VariableDeclaration)

std::ostream& operator<<(std::ostream& stream,
                         const VariableDeclaration& declaration) {
  stream << node_type(declaration) << " " << declaration.name() << " = ("
         << indent << indentation;
  apply_visitor(StreamPrinter(stream), declaration.definition());
  return stream << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
