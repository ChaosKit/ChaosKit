#ifndef CHAOSKIT_AST_EXPRESSION_DEBUG_H
#define CHAOSKIT_AST_EXPRESSION_DEBUG_H

#include <ostream>
#include "BinaryFunction.h"
#include "Expression.h"
#include "Input.h"
#include "Output.h"
#include "Parameter.h"
#include "RandomNumber.h"
#include "UnaryFunction.h"
#include "VariableDeclaration.h"
#include "VariableName.h"

namespace chaoskit::ast {

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

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_EXPRESSION_DEBUG_H
