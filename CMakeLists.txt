cmake_minimum_required(VERSION 3.4)
project(ChaosKit)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty)
add_subdirectory(ast)

add_executable(lol lol.cpp)
target_link_libraries(lol ast)
