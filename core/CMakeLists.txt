add_library(core random.h SimpleInterpreter.h SimpleInterpreter.cpp Point.h Point.cpp)
target_link_libraries(core PUBLIC ast randutils)
