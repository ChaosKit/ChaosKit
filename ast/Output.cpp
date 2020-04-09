#include "Output.h"

#include <magic_enum.hpp>
#include "Input.h"
#include "util.h"

namespace chaoskit::ast {

GENERATE_NODE_TYPE(Output)

std::ostream &operator<<(std::ostream &stream, const Output &output) {
  return stream << node_type(output) << "("
                << magic_enum::enum_name<Output::Type>(output.type()) << ")";
}

}  // namespace chaoskit::ast
