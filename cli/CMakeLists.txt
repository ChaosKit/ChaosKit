find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL)

add_executable(chaoskit-render main.cpp)
target_link_libraries(
  chaoskit-render
  PRIVATE core
          library
          magic_enum
          pcg_random
          ui_core
          ui_storage
          Qt::Core
          Qt::Gui
          Qt::OpenGL)

add_executable(cli_test cli_test.cpp)
target_compile_definitions(
  cli_test
  PRIVATE CHAOSKIT_RENDER_BIN="$<TARGET_FILE:chaoskit-render>"
          CHAOSKIT_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata")
target_link_libraries(cli_test PRIVATE gmock gmock_main Qt::Core Qt::Gui)

add_test(NAME cli_test COMMAND cli_test)
