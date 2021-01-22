# Expression AST
add_library(
  ast_expression
  BinaryFunction.h
  common_debug.cpp
  common_debug.h
  Expression.h
  expression_debug.cpp
  expression_debug.h
  helpers.h
  Input.h
  Output.h
  Parameter.h
  RandomNumber.h
  UnaryFunction.h
  util.h
  VariableName.h)
target_link_libraries(
  ast_expression
  PUBLIC variant
  PRIVATE magic_enum)

# Transform AST
add_library(
  ast
  AffineTransform.h
  BaseMultiTransform.h
  Formula.h
  MultiStepTransform.h
  RandomChoiceTransform.h
  Transform.h
  transform_debug.cpp
  transform_debug.h
  transforms.h
  TransformVariant.h
  TransformWithWeight.h
  VariableDeclaration.h
  WeightedSumTransform.h)
target_link_libraries(
  ast
  PUBLIC ast_expression variant
  PRIVATE magic_enum)
