add_library(core_structures INTERFACE)
target_sources(core_structures INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Blend.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Document.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Formula.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/System.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Transform.h"
        )
target_link_libraries(core_structures INTERFACE ast)
