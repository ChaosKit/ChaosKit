find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickWidgets)

add_executable(ChaosKit WIN32 main.cpp ui.qrc MainWindow.h MainWindow.cpp
        GLToneMapper.cpp GLToneMapper.h TestWindow.cpp TestWindow.h
        HistogramGenerator.cpp HistogramGenerator.h BlenderTask.cpp
        BlenderTask.h System.h Point.h Particle.h GathererTask.cpp
        GathererTask.h HistogramBuffer.h System.cpp SystemView.h SystemView.cpp
        RenderingForm.cpp RenderingForm.h)
set_target_properties(ChaosKit PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(ChaosKit PRIVATE core Qt5::Core Qt5::Gui Qt5::Widgets
        Qt5::Qml Qt5::Quick Qt5::QuickWidgets)

add_custom_target(ChaosKitUI SOURCES Editor.qml MainWindow.ui RenderingForm.ui)
