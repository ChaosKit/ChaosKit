add_library(ast
        AffineTransform.h
        BinaryFunction.h
        Blend.cpp Blend.h
        Expression.h
        Formula.h
        Input.h
        LimitedBlend.cpp LimitedBlend.h
        Output.h
        MultiStepTransform.h
        Node.cpp Node.h
        Parameter.h
        RandomChoiceTransform.h
        RandomNumber.h
        System.cpp System.h
        StaticAffineTransform.cpp StaticAffineTransform.h
        Transform.h
        TransformVariant.h
        TransformWithWeight.h
        UnaryFunction.h
        VariableDeclaration.h
        VariableName.h
        WeightedFormula.cpp WeightedFormula.h
        WeightedSumTransform.h
        ast.h
        debug.cpp debug.h
        helpers.h
        types.h
        util.cpp util.h)
target_link_libraries(ast PUBLIC variant PRIVATE magic_enum)

add_executable(ast_test ast_test.cpp)
target_link_libraries(ast_test PRIVATE gmock_main ast magic_enum)
add_test(NAME ast_test COMMAND ast_test)
