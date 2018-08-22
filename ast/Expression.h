#ifndef CHAOSKIT_AST_EXPRESSION_H
#define CHAOSKIT_AST_EXPRESSION_H

#include <mapbox/variant.hpp>
#include "Input.h"
#include "Parameter.h"

namespace chaoskit {
namespace ast {

class UnaryFunction;
class BinaryFunction;

using Expression =
    mapbox::util::variant<float, Input, Parameter,
                          mapbox::util::recursive_wrapper<UnaryFunction>,
                          mapbox::util::recursive_wrapper<BinaryFunction>>;

}  // namespace ast
}  // namespace chaoskit

#include "BinaryFunction.h"
#include "UnaryFunction.h"

#endif  // CHAOSKIT_AST_EXPRESSION_H
