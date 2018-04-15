#ifndef CHAOSKIT_NODE_H
#define CHAOSKIT_NODE_H

#include <mapbox/variant.hpp>
#include "util.h"

namespace chaoskit {
namespace ast {

class Input;
class Parameter;
class UnaryFunction;
class BinaryFunction;
class Formula;

using Node =
    mapbox::util::variant<float, Input, Parameter,
                          mapbox::util::recursive_wrapper<UnaryFunction>,
                          mapbox::util::recursive_wrapper<BinaryFunction>,
                          mapbox::util::recursive_wrapper<Formula>>;

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_NODE_H
