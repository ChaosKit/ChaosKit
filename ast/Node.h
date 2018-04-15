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

namespace {
using NodeAlias =
    mapbox::util::variant<float, Input, Parameter,
                          mapbox::util::recursive_wrapper<UnaryFunction>,
                          mapbox::util::recursive_wrapper<BinaryFunction>,
                          mapbox::util::recursive_wrapper<Formula>>;
}

struct Node : public NodeAlias {
  using NodeAlias::NodeAlias;
  Node(const Expression& expression);
};

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_NODE_H
