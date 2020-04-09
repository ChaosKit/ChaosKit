add_subdirectory(structures)

add_library(core
        BlackWhiteColorMap.h
        Color.h
        ColorMap.h
        ColorMapRegistry.cpp ColorMapRegistry.h
        DocumentDeleter.cpp DocumentDeleter.h
        DocumentVisitor.h
        errors.cpp errors.h
        HistogramBuffer.h HistogramBuffer.cpp
        ManagedDocument.cpp ManagedDocument.h
        PaletteColorMap.cpp PaletteColorMap.h
        Params.h
        Particle.h
        Point.h Point.cpp
        RainbowColorMap.cpp RainbowColorMap.h
        Rng.h
        SimpleHistogramGenerator.h SimpleHistogramGenerator.cpp
        SimpleInterpreter.h SimpleInterpreter.cpp
        SystemIndex.h
        ThreadLocalRng.h ThreadLocalRng.cpp
        random.h
        toSource.h toSource.cpp
        transforms.cpp transforms.h
        util.h util.cpp)
target_link_libraries(core
        PUBLIC ast stdx randutils
        PRIVATE magic_enum
        INTERFACE core_structures)

add_executable(core_test SimpleInterpreterTest.cpp)
target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
