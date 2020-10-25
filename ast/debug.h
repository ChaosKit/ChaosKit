#ifndef CHAOSKIT_AST_DEBUG_H
#define CHAOSKIT_AST_DEBUG_H

#include <ostream>
#include "AffineTransform.h"
#include "BinaryFunction.h"
#include "Expression.h"
#include "Formula.h"
#include "Input.h"
#include "MultiStepTransform.h"
#include "Output.h"
#include "Parameter.h"
#include "PositionTransform.h"
#include "RandomChoiceTransform.h"
#include "RandomNumber.h"
#include "Transform.h"
#include "UnaryFunction.h"
#include "VariableDeclaration.h"
#include "VariableName.h"
#include "WeightedSumTransform.h"

namespace chaoskit::ast {

// Expressions

std::ostream& operator<<(std::ostream& stream, const Expression& expression);

std::ostream& operator<<(std::ostream& stream, const BinaryFunction& function);
std::ostream& operator<<(std::ostream& stream, const Input& input);
std::ostream& operator<<(std::ostream& stream, const Output& output);
std::ostream& operator<<(std::ostream& stream, const Parameter& parameter);
std::ostream& operator<<(std::ostream& stream, const RandomNumber&);
std::ostream& operator<<(std::ostream& stream, const UnaryFunction& function);
std::ostream& operator<<(std::ostream& stream,
                         const VariableDeclaration& declaration);
std::ostream& operator<<(std::ostream& stream, const VariableName& name);

// Transforms

std::ostream& operator<<(std::ostream& stream,
                         const PositionTransform& transform);
std::ostream& operator<<(std::ostream& stream, const Transform& transform);

std::ostream& operator<<(std::ostream& stream,
                         const AffineTransform& transform);
std::ostream& operator<<(std::ostream& stream, const Formula& formula);
std::ostream& operator<<(std::ostream& stream,
                         const MultiStepTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const RandomChoiceTransform& transform);
std::ostream& operator<<(std::ostream& stream,
                         const WeightedSumTransform& transform);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_DEBUG_H
