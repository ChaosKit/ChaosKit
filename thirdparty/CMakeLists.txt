# Mapbox Variant (https://github.com/mapbox/variant)
add_library(variant INTERFACE)
target_include_directories(variant INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/variant)
target_sources(variant INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/mapbox/optional.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/mapbox/recursive_wrapper.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/mapbox/variant_cast.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/mapbox/variant_io.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/mapbox/variant_visitor.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/variant/mapbox/variant.hpp)
