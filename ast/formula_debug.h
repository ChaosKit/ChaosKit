#ifndef CHAOSKIT_AST_FORMULA_DEBUG_H
#define CHAOSKIT_AST_FORMULA_DEBUG_H

#include <ostream>
#include "Formula.h"

namespace chaoskit::ast {

std::ostream& operator<<(std::ostream& stream, const Formula& formula);

}

#endif  // CHAOSKIT_AST_FORMULA_DEBUG_H
