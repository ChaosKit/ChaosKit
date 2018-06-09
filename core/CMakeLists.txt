add_library(core random.h SimpleInterpreter.h SimpleInterpreter.cpp
        Point.h Point.cpp Rng.h ThreadLocalRng.cpp ThreadLocalRng.h)
target_link_libraries(core PUBLIC ast randutils)

add_executable(core_test SimpleInterpreterTest.cpp)
target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
