cmake_minimum_required(VERSION 3.6)
project(ChaosKit)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
find_package(Qt5 COMPONENTS Core Gui)

enable_testing()
add_subdirectory(stdx)
add_subdirectory(thirdparty)
add_subdirectory(ast)
add_subdirectory(core)
add_subdirectory(state)
add_subdirectory(library)
add_subdirectory(ui)

add_executable(lol lol.cpp)
target_link_libraries(lol PRIVATE core library Qt5::Gui)
