#include "WeightedFormula.h"
#include "formula_debug.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(WeightedFormula);

std::ostream &operator<<(std::ostream &stream, const WeightedFormula &formula) {
  return stream << node_type(formula) << " (" << indent << indentation
                << "weight: [" << formula.weight_x() << ", "
                << formula.weight_y() << "]" << indentation << formula.formula()
                << outdent << indentation << ")";
}

}  // namespace chaoskit::ast
