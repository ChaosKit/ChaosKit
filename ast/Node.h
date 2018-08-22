#ifndef CHAOSKIT_AST_NODE_H
#define CHAOSKIT_AST_NODE_H

#include <mapbox/variant.hpp>
#include "BinaryFunction.h"
#include "Expression.h"
#include "Input.h"
#include "Parameter.h"
#include "Transform.h"
#include "UnaryFunction.h"

namespace chaoskit {
namespace ast {

class Formula;
class WeightedFormula;
class Blend;
class LimitedBlend;
class System;

namespace {
using NodeAlias =
    mapbox::util::variant<float, Input, Parameter, Transform,
                          mapbox::util::recursive_wrapper<UnaryFunction>,
                          mapbox::util::recursive_wrapper<BinaryFunction>,
                          mapbox::util::recursive_wrapper<Formula>,
                          mapbox::util::recursive_wrapper<WeightedFormula>,
                          mapbox::util::recursive_wrapper<Blend>,
                          mapbox::util::recursive_wrapper<LimitedBlend>,
                          mapbox::util::recursive_wrapper<System>>;
}

struct Node : public NodeAlias {
  using NodeAlias::NodeAlias;
  Node(const Expression& expression);
};

}  // namespace ast
}  // namespace chaoskit

#include "Formula.h"
#include "WeightedFormula.h"
#include "Blend.h"
#include "LimitedBlend.h"
#include "System.h"

#endif  // CHAOSKIT_AST_NODE_H
