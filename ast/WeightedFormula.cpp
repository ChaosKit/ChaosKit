#include "WeightedFormula.h"
#include "util.h"

namespace chaoskit {
namespace ast {

GENERATE_NODE_TYPE(WeightedFormula);

std::ostream &operator<<(std::ostream &stream, const WeightedFormula &formula) {
  return stream << node_type(formula) << "(" << formula.formula() << ", "
                << formula.weight_x() << ", " << formula.weight_y()
                << ")";
}

}  // namespace ast
}  // namespace chaoskit
