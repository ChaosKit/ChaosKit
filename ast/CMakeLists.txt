add_library(
  ast
  AffineTransform.h
  ast.h
  BinaryFunction.h
  Blend.cpp
  Blend.h
  debug.cpp
  debug.h
  Expression.h
  Formula.h
  helpers.h
  Input.h
  LimitedBlend.cpp
  LimitedBlend.h
  MultiStepTransform.h
  Node.cpp
  Node.h
  Output.h
  Parameter.h
  RandomChoiceTransform.h
  RandomNumber.h
  StaticAffineTransform.cpp
  StaticAffineTransform.h
  System.cpp
  System.h
  Transform.h
  transforms.h
  TransformVariant.h
  TransformWithWeight.h
  types.h
  UnaryFunction.h
  util.cpp
  util.h
  VariableDeclaration.h
  VariableName.h
  WeightedFormula.cpp
  WeightedFormula.h
  WeightedSumTransform.h)
target_link_libraries(
  ast
  PUBLIC variant
  PRIVATE magic_enum)

add_executable(ast_test ast_test.cpp)
target_link_libraries(ast_test PRIVATE gmock_main ast magic_enum)
add_test(NAME ast_test COMMAND ast_test)
