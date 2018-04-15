add_library(library INTERFACE)
target_link_libraries(library INTERFACE ast)
target_sources(library INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Formula.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DeJong.h)
