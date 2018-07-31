add_library(core random.h SimpleInterpreter.h SimpleInterpreter.cpp
        Point.h Point.cpp Rng.h ThreadLocalRng.cpp ThreadLocalRng.h System.h
        Formula.h Blend.h Transform.h SystemIndex.h Params.h util.cpp util.h
        SimpleBlender.h SimpleBlender.cpp HistogramBuffer.cpp HistogramBuffer.h HistogramColor.h)
target_link_libraries(core PUBLIC ast stdx randutils)

add_executable(core_test SimpleInterpreterTest.cpp)
target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
