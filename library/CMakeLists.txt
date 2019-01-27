add_library(library INTERFACE)
target_link_libraries(library INTERFACE ast better_enums)
target_sources(library INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Formula.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FormulaType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DeJong.h
        ${CMAKE_CURRENT_SOURCE_DIR}/util.h)
