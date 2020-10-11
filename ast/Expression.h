#ifndef CHAOSKIT_AST_EXPRESSION_H
#define CHAOSKIT_AST_EXPRESSION_H

#include <mapbox/variant.hpp>
#include "Input.h"
#include "Output.h"
#include "Parameter.h"
#include "VariableName.h"

namespace chaoskit::ast {

class UnaryFunction;
class BinaryFunction;

using Expression =
    mapbox::util::variant<float, Input, Output, Parameter, VariableName,
                          mapbox::util::recursive_wrapper<UnaryFunction>,
                          mapbox::util::recursive_wrapper<BinaryFunction>>;

}  // namespace chaoskit::ast

#include "BinaryFunction.h"
#include "UnaryFunction.h"

#endif  // CHAOSKIT_AST_EXPRESSION_H
