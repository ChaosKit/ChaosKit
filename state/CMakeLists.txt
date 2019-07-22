add_library(state INTERFACE)
target_sources(state INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/hierarchy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Store.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Id.h
        )
target_link_libraries(state INTERFACE variant)

add_executable(state_test
        hierarchy_test.cpp
        Id_test.cpp
        Store_test.cpp
        )
target_link_libraries(state_test PRIVATE gmock_main state)
add_test(NAME state_test COMMAND state_test)
