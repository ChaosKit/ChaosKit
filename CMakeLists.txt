cmake_minimum_required(VERSION 3.14)
project(ChaosKit)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

# Allow executables to access DLL dependencies on Windows.
if (WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_compile_definitions(_USE_MATH_DEFINES) # Needed for MSVC

enable_testing()
add_subdirectory(stdx)
add_subdirectory(ast)
add_subdirectory(core)
add_subdirectory(flame)
add_subdirectory(state)
add_subdirectory(library)
add_subdirectory(io)
add_subdirectory(ui)

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE ast core randutils)

find_package(Qt5 COMPONENTS Core Gui)
add_executable(lol lol.cpp)
target_link_libraries(lol PRIVATE core flame library state Qt5::Gui)

####################################################### Third party dependencies

include(FetchContent)

# Mapbox Variant (https://github.com/mapbox/variant)
FetchContent_Declare(
  variant
  GIT_REPOSITORY https://github.com/mapbox/variant.git
  GIT_TAG v1.2.0
)
FetchContent_GetProperties(variant)
if (NOT variant_POPULATED)
  FetchContent_Populate(variant)

  add_library(variant INTERFACE)
  target_include_directories(variant INTERFACE ${variant_SOURCE_DIR}/include)
  target_sources(variant INTERFACE
    ${variant_SOURCE_DIR}/include/mapbox/optional.hpp
    ${variant_SOURCE_DIR}/include/mapbox/recursive_wrapper.hpp
    ${variant_SOURCE_DIR}/include/mapbox/variant_cast.hpp
    ${variant_SOURCE_DIR}/include/mapbox/variant_io.hpp
    ${variant_SOURCE_DIR}/include/mapbox/variant_visitor.hpp
    ${variant_SOURCE_DIR}/include/mapbox/variant.hpp)
endif ()

# randutils.hpp (https://gist.github.com/imneme/540829265469e673d045)
FetchContent_Declare(
  randutils
  GIT_REPOSITORY https://gist.github.com/540829265469e673d045.git
)
FetchContent_GetProperties(randutils)
if (NOT randutils_POPULATED)
  FetchContent_Populate(randutils)

  add_library(randutils INTERFACE)
  target_include_directories(randutils INTERFACE ${randutils_SOURCE_DIR})
  target_sources(randutils INTERFACE ${randutils_SOURCE_DIR}/randutils.hpp)
endif ()


# Google Test (https://github.com/google/googletest)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG df7fee587d442b372ef43bd66c6a2f5c9af8c5eb
)
FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  # Prevent overriding the parent project's compiler/linker settings on Windows
  set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
  # Prevent RPATH warnings
  set(CMAKE_MACOSX_RPATH 1)
  add_subdirectory(${googletest_SOURCE_DIR} EXCLUDE_FROM_ALL)
endif ()
FetchContent_MakeAvailable(googletest)

# Magic Enum (https://github.com/Neargye/magic_enum)
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG v0.7.2
)
FetchContent_MakeAvailable(magic_enum)

# xxHash (https://github.com/Cyan4973/xxHash)
FetchContent_Declare(
  xxhash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
  GIT_TAG v0.8.0
)
FetchContent_GetProperties(xxhash)
if (NOT xxhash_POPULATED)
  FetchContent_Populate(xxhash)
  set(XXHASH_BUILD_XXHSUM OFF)
  add_subdirectory(${xxhash_SOURCE_DIR}/cmake_unofficial EXCLUDE_FROM_ALL)
endif ()
