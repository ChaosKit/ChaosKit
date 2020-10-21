add_library(ast
        BinaryFunction.cpp BinaryFunction.h
        Blend.cpp Blend.h
        Expression.h
        Formula.cpp Formula.h
        Input.cpp Input.h
        LimitedBlend.cpp LimitedBlend.h
        Output.cpp Output.h
        Node.cpp Node.h
        Parameter.cpp Parameter.h
        System.cpp System.h
        StaticAffineTransform.cpp StaticAffineTransform.h
        UnaryFunction.cpp UnaryFunction.h
        VariableDeclaration.cpp VariableDeclaration.h
        VariableName.cpp VariableName.h
        WeightedFormula.cpp WeightedFormula.h
        ast.h
        helpers.h
        types.h
        util.cpp util.h RandomNumber.h RandomNumber.cpp)
target_link_libraries(ast PUBLIC variant PRIVATE magic_enum)

add_executable(ast_test ast_test.cpp)
target_link_libraries(ast_test PRIVATE gmock_main ast magic_enum)
add_test(NAME ast_test COMMAND ast_test)
