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
  VariableName.h)
target_link_libraries(
  ast_expression
  PUBLIC variant
  PRIVATE magic_enum)

# Flame-oriented AST - deprecated
add_library(
  ast
  Blend.cpp
  Blend.h
  flame_helpers.h
  Formula.h # used in both flame and transform ASTs
  formula_debug.cpp
  formula_debug.h
  LimitedBlend.cpp
  LimitedBlend.h
  Node.cpp
  Node.h
  StaticAffineTransform.cpp
  StaticAffineTransform.h
  System.cpp
  System.h
  util.cpp
  util.h
  VariableDeclaration.h # used in both flame and transform ASTs
  WeightedFormula.cpp
  WeightedFormula.h)
target_link_libraries(ast PUBLIC ast_expression variant)

# Transform-oriented AST
add_library(
  ast_transform
  AffineTransform.h
  Formula.h # used in both flame and transform ASTs
  formula_debug.cpp
  formula_debug.h
  MultiStepTransform.h
  RandomChoiceTransform.h
  Transform.h
  transform_debug.cpp
  transform_debug.h
  transforms.h
  TransformVariant.h
  TransformWithWeight.h
  VariableDeclaration.h # used in both flame and transform ASTs
  WeightedSumTransform.h)
target_link_libraries(
  ast_transform
  PUBLIC ast_expression variant
  PRIVATE magic_enum)

add_executable(ast_test ast_test.cpp)
target_link_libraries(ast_test PRIVATE gmock_main ast magic_enum)
add_test(NAME ast_test COMMAND ast_test)
