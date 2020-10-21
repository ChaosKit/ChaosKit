#ifndef CHAOSKIT_AST_NODE_H
#define CHAOSKIT_AST_NODE_H

#include <mapbox/variant.hpp>
#include "BinaryFunction.h"
#include "Expression.h"
#include "Input.h"
#include "Output.h"
#include "Parameter.h"
#include "RandomNumber.h"
#include "StaticAffineTransform.h"
#include "UnaryFunction.h"
#include "VariableName.h"

namespace chaoskit::ast {

class VariableDeclaration;
class Formula;
class WeightedFormula;
class Blend;
class LimitedBlend;
class System;

namespace {
using NodeAlias =
    mapbox::util::variant<float, Input, Output, Parameter,
                          StaticAffineTransform, VariableName, RandomNumber,
                          mapbox::util::recursive_wrapper<VariableDeclaration>,
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
  // This intentionally allows to implicitly convert from an Expression to a
  // Node for convenience.
  // NOLINTNEXTLINE(hicpp-explicit-conversions, google-explicit-constructor)
  Node(const Expression& expression);
};

}  // namespace chaoskit::ast

#include "Blend.h"
#include "Formula.h"
#include "LimitedBlend.h"
#include "System.h"
#include "VariableDeclaration.h"
#include "WeightedFormula.h"

#endif  // CHAOSKIT_AST_NODE_H
