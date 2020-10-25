#ifndef CHAOSKIT_AST_DEBUG_H
#define CHAOSKIT_AST_DEBUG_H

#include <ostream>
#include "AffineTransform.h"
#include "MultiStepTransform.h"
#include "PositionTransform.h"
#include "RandomChoiceTransform.h"
#include "Transform.h"
#include "WeightedSumTransform.h"

namespace chaoskit::ast {

std::ostream& operator<<(std::ostream& stream,
                         const AffineTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const MultiStepTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const PositionTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const RandomChoiceTransform& transform);
std::ostream& operator<<(std::ostream& stream, const Transform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const WeightedSumTransform& transform);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_DEBUG_H
