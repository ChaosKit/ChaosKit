#ifndef CHAOSKIT_AST_COMMON_DEBUG_H
#define CHAOSKIT_AST_COMMON_DEBUG_H

#include <ostream>

namespace chaoskit::ast {

std::ios_base &indent(std::ios_base &os);
std::ios_base &outdent(std::ios_base &os);
std::ostream &nl(std::ostream &os);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_COMMON_DEBUG_H
