find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickWidgets QuickControls2)

add_subdirectory(models)

add_executable(ChaosKit WIN32
        main.cpp
        ui.qrc
        GLToneMapper.cpp GLToneMapper.h
        TestWindow.cpp TestWindow.h
        HistogramGenerator.cpp HistogramGenerator.h
        BlenderTask.cpp BlenderTask.h
        Point.h
        Particle.h
        GathererTask.cpp GathererTask.h
        HistogramBuffer.h
        SystemView.h SystemView.cpp
        SystemModel.cpp SystemModel.h
        FlatteningModel.cpp FlatteningModel.h)
set_target_properties(ChaosKit PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(ChaosKit PRIVATE
        core
        library
        ChaosKit_Models
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Qml Qt5::Quick Qt5::QuickWidgets
        Qt5::QuickControls2
        )
