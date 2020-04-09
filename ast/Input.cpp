#include "Input.h"

#include <magic_enum.hpp>
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(Input)

std::ostream &operator<<(std::ostream &stream, const Input &input) {
  return stream << node_type(input) << "("
                << magic_enum::enum_name<Input::Type>(input.type()) << ")";
}

}  // namespace chaoskit::ast
