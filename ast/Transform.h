#ifndef CHAOSKIT_AST_TRANSFORM_H
#define CHAOSKIT_AST_TRANSFORM_H

#include <mapbox/variant.hpp>
#include "Formula.h"

namespace chaoskit::ast {

class WeightedSumTransform;

using Transform = mapbox::util::variant<
    Formula, mapbox::util::recursive_wrapper<WeightedSumTransform>>;

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_TRANSFORM_H
