find_package(Qt5 REQUIRED
        COMPONENTS
        Core Gui Qml Quick QuickControls2
        OPTIONAL_COMPONENTS Widgets)

# Library that deals with interfacing with the core.
add_library(ui_core
        BlenderTask.cpp BlenderTask.h
        GathererTask.cpp GathererTask.h
        GLToneMapper.cpp GLToneMapper.h
        HistogramBuffer.h
        HistogramGenerator.cpp HistogramGenerator.h
        Point.h
        Particle.h
        )
set_target_properties(ui_core PROPERTIES AUTOMOC ON)
target_link_libraries(ui_core PUBLIC
        core
        Qt5::Core Qt5::Gui
        )

# The Qt Model interface to chaoskit::state.
add_library(ui_state
        DocumentModel.cpp DocumentModel.h
        ModelEntry.cpp ModelEntry.h
        RandomizationSettings.cpp RandomizationSettings.h
        DocumentProxy.cpp DocumentProxy.h)
set_target_properties(ui_state PROPERTIES AUTOMOC ON)
target_link_libraries(ui_state PUBLIC
        core
        library
        state
        Qt5::Core Qt5::Qml
        )

##### Static resource management

# Except for fonts, we only want to compile resources with QRC in release
# build. This requires additional setup handled in the "ui_resources" library.
set(RESOURCES_SOURCE_FILES resources.cpp resources.h)
if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    qt5_add_resources(RESOURCES_SOURCE_FILES resources.qrc)
endif ()

add_library(ui_resources ${RESOURCES_SOURCE_FILES})
target_link_libraries(ui_resources PUBLIC Qt5::Core)
target_compile_definitions(ui_resources
        PRIVATE
        $<IF:$<CONFIG:Debug>,CHAOSKIT_RESOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR},CHAOSKIT_USE_RESOURCES>
        )

##### Main executable

set(ChaosKit_SOURCES
        main.cpp
        EngineManager.cpp EngineManager.h
        SystemView.h SystemView.cpp
        FormulaPreviewProvider.cpp FormulaPreviewProvider.h
        Utilities.cpp Utilities.h)

# As an exception to the not-always-static resources defined above, custom
# fonts are always embedded in the binary.
qt5_add_resources(ChaosKit_SOURCES static.qrc)

add_executable(ChaosKit WIN32 ${ChaosKit_SOURCES})
set_target_properties(ChaosKit PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC OFF
        )
target_link_libraries(ChaosKit PRIVATE
        core
        library
        ui_core
        ui_resources
        ui_state
        Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick Qt5::QuickControls2
        )

##### run.sh — runs ChaosKit with correct environment variables

add_custom_command(
        TARGET ChaosKit POST_BUILD
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/make_run.sh"
        ARGS $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}>
        BYPRODUCTS run.sh
        COMMENT "Generating run.sh"
)

##### Various experimental testing executables

add_executable(TestWindow WIN32
        test.cpp
        TestWindow.cpp TestWindow.h
        )
set_target_properties(TestWindow PROPERTIES AUTOMOC ON)
target_link_libraries(TestWindow PRIVATE
        core
        library
        ui_core
        ui_state
        Qt5::Core Qt5::Gui Qt5::Widgets
        )
