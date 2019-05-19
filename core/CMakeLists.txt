add_subdirectory(structures)

add_library(core
        HistogramBuffer.h HistogramBuffer.cpp
        HistogramColor.h
        Params.h
        Particle.h
        Point.h Point.cpp
        Rng.h
        SimpleBlender.h SimpleBlender.cpp
        SimpleInterpreter.h SimpleInterpreter.cpp
        SystemIndex.h
        ThreadLocalRng.h ThreadLocalRng.cpp
        random.h
        toSource.h toSource.cpp
        util.h util.cpp)
target_link_libraries(core PUBLIC ast stdx randutils INTERFACE core_structures)

add_executable(core_test SimpleInterpreterTest.cpp)
target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
