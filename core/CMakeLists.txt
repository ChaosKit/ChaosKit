add_library(core random.h SimpleInterpreter.h SimpleInterpreter.cpp
        Point.h Point.cpp Rng.h ThreadLocalRng.cpp ThreadLocalRng.h)
target_link_libraries(core PUBLIC ast randutils)
