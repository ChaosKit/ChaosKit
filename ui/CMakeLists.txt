find_package(Qt5 REQUIRED
        COMPONENTS
        Core Gui Qml Quick QuickControls2
        OPTIONAL_COMPONENTS Widgets)
find_package(KF5ItemModels)

add_subdirectory(models)

add_library(ui_common
        BlenderTask.cpp BlenderTask.h
        GathererTask.cpp GathererTask.h
        GLToneMapper.cpp GLToneMapper.h
        HistogramBuffer.h
        HistogramGenerator.cpp HistogramGenerator.h
        Point.h
        Particle.h
        )
set_target_properties(ui_common PROPERTIES AUTOMOC ON)
target_link_libraries(ui_common PUBLIC
        core
        ui_models
        Qt5::Core Qt5::Gui
        )

add_library(ui_state
        DocumentModel.cpp DocumentModel.h
        )
set_target_properties(ui_state PROPERTIES AUTOMOC ON)
target_link_libraries(ui_state PUBLIC
        core
        library
        state
        Qt5::Core
        )

add_executable(ChaosKit WIN32
        main.cpp
        ui.qrc
        SystemView.h SystemView.cpp
        SystemModel.cpp SystemModel.h
        SubtreeModel.cpp SubtreeModel.h
        SystemElement.cpp SystemElement.h)
set_target_properties(ChaosKit PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(ChaosKit PRIVATE
        core
        library
        ui_common
        ui_models
        Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick Qt5::QuickControls2
        )

add_executable(ModelTest WIN32
        modeltest.cpp
        ModelTestWindow.cpp ModelTestWindow.h
        )
set_target_properties(ModelTest PROPERTIES AUTOMOC ON)
target_link_libraries(ModelTest PRIVATE
        core
        ui_state
        Qt5::Core Qt5::Gui Qt5::Widgets
        KF5::ItemModels
        )

add_executable(TestWindow WIN32
        test.cpp
        TestWindow.cpp TestWindow.h
        )
set_target_properties(TestWindow PROPERTIES AUTOMOC ON)
target_link_libraries(TestWindow PRIVATE
        core
        library
        ui_common
        ui_state
        Qt5::Core Qt5::Gui Qt5::Widgets
        )
