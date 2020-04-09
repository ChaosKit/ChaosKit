# Automatic submodule update
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Update submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Updating Git submodules")
        execute_process(COMMAND
                ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR
                    "git submodule update --init failed with \
                    ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif ()
    endif ()
endif ()

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/googletest/CMakeLists.txt")
    message(FATAL_ERROR
            "The submodules were not downloaded! GIT_SUBMODULE was turned off \
            or failed. Please update submodules and try again.")
endif ()

# Mapbox Variant (https://github.com/mapbox/variant)
add_library(variant INTERFACE)
target_include_directories(variant INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/include)
target_sources(variant INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/optional.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/recursive_wrapper.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant_cast.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant_io.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant_visitor.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/mapbox/variant.hpp)

# randutils.hpp (https://gist.github.com/imneme/540829265469e673d045)
add_library(randutils INTERFACE)
target_include_directories(randutils INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/randutils)
target_sources(randutils INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/randutils/randutils.hpp)

# Google Test (https://github.com/google/googletest)

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)

# Magic Enum (https://github.com/Neargye/magic_enum)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/magic_enum)
