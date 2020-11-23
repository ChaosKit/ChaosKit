# Mapbox Variant (https://github.com/mapbox/variant)
add_library(variant INTERFACE)
target_include_directories(
  variant INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/variant/include)
target_sources(
  variant
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/optional.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/recursive_wrapper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant_cast.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant_io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant_visitor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant.hpp)

if(MSVC)
  target_compile_definitions(
    variant INTERFACE _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)
endif()

# randutils.hpp (https://gist.github.com/imneme/540829265469e673d045)
add_library(randutils INTERFACE)
target_include_directories(randutils
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/randutils)
target_sources(randutils
               INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/randutils/randutils.hpp)

# Google Test (https://github.com/google/googletest)

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)

# Magic Enum (https://github.com/Neargye/magic_enum)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/magic_enum)
