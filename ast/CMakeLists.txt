add_library(ast Node.cpp ast.h helpers.h util.h Expression.h Node.h Input.h
        UnaryFunction.h BinaryFunction.h Formula.h Parameter.h types.h Input.cpp
        Parameter.cpp UnaryFunction.cpp BinaryFunction.cpp Formula.cpp Blend.h
        Blend.cpp WeightedFormula.cpp WeightedFormula.h Transform.cpp
        Transform.h LimitedBlend.cpp LimitedBlend.h System.cpp System.h)
target_link_libraries(ast PUBLIC variant better_enums)

add_executable(ast_test ast_test.cpp)
target_link_libraries(ast_test PRIVATE gmock_main ast)
add_test(NAME ast_test COMMAND ast_test)
