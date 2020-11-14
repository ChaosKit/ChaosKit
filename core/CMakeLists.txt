add_library(core
        BlackWhiteColorMap.h
        Color.h
        ColorMap.h
        ColorMapRegistry.cpp ColorMapRegistry.h
        DocumentDeleter.cpp DocumentDeleter.h
        DocumentVisitor.h
        errors.cpp errors.h
        ExpressionInterpreter.cpp ExpressionInterpreter.h
        Generator.h
        HistogramBuffer.h HistogramBuffer.cpp
        ManagedDocument.cpp ManagedDocument.h
        PaletteColorMap.cpp PaletteColorMap.h
        Params.h
        Particle.h
        Point.h
        RainbowColorMap.cpp RainbowColorMap.h
        Renderer.h
        Rng.h
        SimpleHistogramGenerator.h SimpleHistogramGenerator.cpp
        SimpleInterpreter.h SimpleInterpreter.cpp
        SystemProcessor.cpp SystemProcessor.h
        SystemIndex.h
        SystemParticle.h
        ThreadLocalRng.h ThreadLocalRng.cpp
        TransformIndex.cpp TransformIndex.h
        TransformInterpreter.cpp TransformInterpreter.h
        TransformParams.h
        TransformSystem.h
        debug.cpp debug.h
        random.h
        toSource.h toSource.cpp
        transforms.cpp transforms.h
        util.h util.cpp)
target_link_libraries(core
        PUBLIC ast stdx randutils
        PRIVATE magic_enum xxHash::xxhash
        INTERFACE flame_structures)

add_executable(core_test
        testing/MockRng.h
        testing/StaticRng.h
        ExpressionInterpreterTest.cpp
        SimpleInterpreterTest.cpp
        SystemProcessorTest.cpp
        TransformIndexTest.cpp
        TransformInterpreterTest.cpp)

target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
