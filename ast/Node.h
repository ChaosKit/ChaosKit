#ifndef CHAOSKIT_NODE_H
#define CHAOSKIT_NODE_H

#include <mapbox/variant.hpp>
#include "Input.h"
#include "Parameter.h"
#include "Expression.h"

namespace chaoskit {
namespace ast {

class UnaryFunction;
class BinaryFunction;
class Formula;
class WeightedFormula;
class Blend;

namespace {
using NodeAlias =
    mapbox::util::variant<float, Input, Parameter,
                          mapbox::util::recursive_wrapper<UnaryFunction>,
                          mapbox::util::recursive_wrapper<BinaryFunction>,
                          mapbox::util::recursive_wrapper<Formula>,
                          mapbox::util::recursive_wrapper<WeightedFormula>,
                          mapbox::util::recursive_wrapper<Blend>>;
}

struct Node : public NodeAlias {
  using NodeAlias::NodeAlias;
  Node(const Expression& expression);
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_NODE_H
