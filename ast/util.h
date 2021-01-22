#ifndef CHAOSKIT_AST_UTIL_H
#define CHAOSKIT_AST_UTIL_H

#include <mapbox/variant.hpp>
#include <mapbox/variant_cast.hpp>

namespace chaoskit::ast {

using mapbox::util::apply_visitor;
using mapbox::util::static_variant_cast;

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_UTIL_H
