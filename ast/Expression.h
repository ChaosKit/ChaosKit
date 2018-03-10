#ifndef CHAOSKIT_EXPRESSION_H
#define CHAOSKIT_EXPRESSION_H

#include <mapbox/variant.hpp>

namespace chaoskit {
namespace ast {

class Input;
class Parameter;
class UnaryFunction;
class BinaryFunction;

using Expression =
    mapbox::util::variant<float, Input, Parameter,
                          mapbox::util::recursive_wrapper<UnaryFunction>,
                          mapbox::util::recursive_wrapper<BinaryFunction>>;

}  // namespace ast
}  // namespace chaoskit

#endif  // CHAOSKIT_EXPRESSION_H
