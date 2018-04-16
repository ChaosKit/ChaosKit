cmake_minimum_required(VERSION 3.4)
project(ChaosKit)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

enable_testing()
add_subdirectory(thirdparty)
add_subdirectory(ast)
add_subdirectory(library)

add_executable(lol lol.cpp)
target_link_libraries(lol PRIVATE ast library)

# Tests
add_executable(lol_test lol_test.cpp)
target_link_libraries(lol_test PRIVATE gtest_main)
add_test(NAME lol_test COMMAND lol_test)
