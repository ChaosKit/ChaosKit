find_package(
  Qt5 REQUIRED
  COMPONENTS Core Gui Qml Quick QuickControls2
  OPTIONAL_COMPONENTS Widgets)

add_subdirectory(models)
add_subdirectory(storage)

# Library that deals with interfacing with the core.
add_library(
  ui_core
  BlenderTask.cpp
  BlenderTask.h
  GathererTask.cpp
  GathererTask.h
  GLToneMapper.cpp
  GLToneMapper.h
  HistogramBuffer.h
  HistogramGenerator.cpp
  HistogramGenerator.h
  Particle.h
  Point.h)
set_target_properties(ui_core PROPERTIES AUTOMOC ON)
target_link_libraries(ui_core PUBLIC core flame Qt5::Core Qt5::Gui)

# --- Static resource management ---

# Except for fonts, we only want to compile resources with QRC in release build.
# This requires additional setup handled in the "ui_resources" library.
set(RESOURCES_SOURCE_FILES resources.cpp resources.h)
if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  qt5_add_resources(RESOURCES_SOURCE_FILES resources.qrc)
endif ()

add_library(ui_resources ${RESOURCES_SOURCE_FILES})
target_link_libraries(ui_resources PUBLIC Qt5::Core)
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
  SystemView.h
  SystemView.cpp
  FormulaPreviewProvider.cpp
  FormulaPreviewProvider.h
  Utilities.cpp
  Utilities.h)

# As an exception to the not-always-static resources defined above, custom fonts
# are always embedded in the binary.
qt5_add_resources(ChaosKit_SOURCES static.qrc)

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
  ui_colormapregistry
  ui_models
  ui_resources
  ui_storage
  magic_enum
  qt_qml_models
  Qt5::Core
  Qt5::Gui
  Qt5::Qml
  Qt5::Quick
  Qt5::QuickControls2)
