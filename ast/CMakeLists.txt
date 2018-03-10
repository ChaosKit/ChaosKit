add_library(ast INTERFACE)
target_include_directories(ast INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(ast INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/ast.h
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/util.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Expression.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Node.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Input.h
        ${CMAKE_CURRENT_SOURCE_DIR}/UnaryFunction.h
        ${CMAKE_CURRENT_SOURCE_DIR}/BinaryFunction.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.h)
target_link_libraries(ast INTERFACE variant)
