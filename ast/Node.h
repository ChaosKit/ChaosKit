#ifndef CHAOSKIT_NODE_H
#define CHAOSKIT_NODE_H

#include <mapbox/variant.hpp>

namespace chaoskit {
namespace ast {

class Input;
class Number;
class Parameter;
class UnaryFunction;

using Node =
    mapbox::util::variant<Input, Number, Parameter,
                          mapbox::util::recursive_wrapper<UnaryFunction>>;

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_NODE_H
