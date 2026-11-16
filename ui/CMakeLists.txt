find_package(
  Qt6 REQUIRED
  COMPONENTS Core
             Gui
             GuiPrivate
             Qml
             Quick
             QuickControls2
             ShaderTools
             ShaderToolsPrivate
             Test
  OPTIONAL_COMPONENTS Widgets)

add_subdirectory(models)
add_subdirectory(storage)

# Library that deals with interfacing with the core.
add_library(
  ui_core
  CpuAccumulatorDriver.cpp
  CpuAccumulatorDriver.h
  RhiOffscreenContext.cpp
  RhiOffscreenContext.h
  RhiShaderBaker.cpp
  RhiShaderBaker.h
  RhiToneMapper.cpp
  RhiToneMapper.h)
set_target_properties(ui_core PROPERTIES AUTOMOC ON)
target_link_libraries(ui_core PUBLIC core Qt::Core Qt::Gui Qt6::GuiPrivate
                                     Qt6::ShaderToolsPrivate)

add_library(ui_view SystemView.cpp SystemView.h)
set_target_properties(ui_view PROPERTIES AUTOMOC ON)
target_link_libraries(
  ui_view
  PUBLIC core
         ui_core
         ui_models
         ui_storage
         Qt::Core
         Qt::Gui
         Qt::Quick)

add_executable(ui_driver_test CpuAccumulatorDriverTest.cpp)
set_target_properties(ui_driver_test PROPERTIES AUTOMOC ON)
target_link_libraries(ui_driver_test PRIVATE gmock gmock_main ui_core Qt::Core
                                             Qt::Test)
add_test(NAME ui_driver_test COMMAND ui_driver_test)

add_executable(ui_test SystemViewTest.cpp)
set_target_properties(ui_test PROPERTIES AUTOMOC ON)
target_link_libraries(
  ui_test
  PRIVATE gmock
          core
          library
          ui_colormapregistry
          ui_models
          ui_storage
          ui_view
          Qt::Core
          Qt::Gui
          Qt::Quick
          Qt::Test)
add_test(NAME ui_test COMMAND ui_test)
set_tests_properties(ui_test PROPERTIES LABELS "gpu")

# --- Static resource management ---

# Except for fonts, we only want to compile resources with QRC in release build.
# This requires additional setup handled in the "ui_resources" library.
set(RESOURCES_SOURCE_FILES resources.cpp resources.h)
if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  qt_add_resources(RESOURCES_SOURCE_FILES resources.qrc)
endif()

add_library(ui_resources ${RESOURCES_SOURCE_FILES})
target_link_libraries(ui_resources PUBLIC Qt::Core)
target_compile_definitions(
  ui_resources
  PRIVATE
    $<IF:$<CONFIG:Debug>,CHAOSKIT_RESOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR},CHAOSKIT_USE_RESOURCES>
)

# --- Main executable ---

set(ChaosKit_SOURCES
    main.cpp
    ColorMap.h
    ColorMapPreviewProvider.cpp
    ColorMapPreviewProvider.h
    EngineManager.cpp
    EngineManager.h
    FormulaPreviewProvider.cpp
    FormulaPreviewProvider.h
    Utilities.cpp
    Utilities.h)

# As an exception to the not-always-static resources defined above, custom fonts
# are always embedded in the binary.
qt_add_resources(ChaosKit_SOURCES static.qrc)

add_executable(ChaosKit WIN32 ${ChaosKit_SOURCES})
set_target_properties(
  ChaosKit
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC OFF)
target_link_libraries(
  ChaosKit
  PRIVATE core
          library
          ui_core
          ui_view
          ui_colormapregistry
          ui_models
          ui_resources
          ui_storage
          magic_enum
          qt_qml_models
          Qt::Core
          Qt::Gui
          Qt::Qml
          Qt::Quick
          Qt::QuickControls2)
