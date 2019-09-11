add_subdirectory(structures)
add_subdirectory(models)

add_library(core
        errors.cpp errors.h
        HistogramBuffer.h HistogramBuffer.cpp
        HistogramColor.h
        Params.h
        Particle.h
        Point.h Point.cpp
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
        PUBLIC ast stdx randutils core_models
        INTERFACE core_structures)

add_executable(core_test SimpleInterpreterTest.cpp)
target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
