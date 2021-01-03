#ifndef CHAOSKIT_AST_TRANSFORM_DEBUG_H
#define CHAOSKIT_AST_TRANSFORM_DEBUG_H

#include <ostream>
#include "AffineTransform.h"
#include "MultiStepTransform.h"
#include "RandomChoiceTransform.h"
#include "Transform.h"
#include "TransformVariant.h"
#include "WeightedSumTransform.h"
#include "formula_debug.h"

namespace chaoskit::ast {

std::ostream& operator<<(std::ostream& stream,
                         const TransformVariant& transform);
std::ostream& operator<<(std::ostream& stream, const Transform& transform);

std::ostream& operator<<(std::ostream& stream,
                         const AffineTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const MultiStepTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const RandomChoiceTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const WeightedSumTransform& transform);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_TRANSFORM_DEBUG_H