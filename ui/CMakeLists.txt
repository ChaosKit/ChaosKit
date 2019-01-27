find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickWidgets QuickControls2)

add_subdirectory(models)

add_executable(ChaosKit WIN32
        main.cpp
        ui.qrc
        MainWindow.h MainWindow.cpp
        GLToneMapper.cpp GLToneMapper.h
        TestWindow.cpp TestWindow.h
        HistogramGenerator.cpp HistogramGenerator.h
        BlenderTask.cpp BlenderTask.h
        System.h System.cpp
        Point.h
        Particle.h
        GathererTask.cpp GathererTask.h
        HistogramBuffer.h
        SystemView.h SystemView.cpp
        RenderingForm.cpp RenderingForm.h
        SystemModel.cpp SystemModel.h
        StructureForm.cpp StructureForm.h
        EmptyOptionsForm.cpp EmptyOptionsForm.h
        FlatteningModel.cpp FlatteningModel.h)
set_target_properties(ChaosKit PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(ChaosKit PRIVATE
        core
        library
        ChaosKit_Models
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Qml Qt5::Quick Qt5::QuickWidgets
        Qt5::QuickControls2
        )

add_custom_target(ChaosKitUI SOURCES
        Editor.qml
        MainWindow.qml
        MainWindow.ui
        RenderingForm.ui
        StructureForm.ui
        EmptyOptionsForm.ui
)
