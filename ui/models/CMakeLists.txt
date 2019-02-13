add_library(ChaosKit_Models
        Formula.h Formula.cpp
        Blend.cpp Blend.h
        System.cpp System.h)
set_target_properties(ChaosKit_Models PROPERTIES AUTOMOC ON)
target_link_libraries(ChaosKit_Models PUBLIC
        core library
        Qt5::Core
        Qt5::Gui)
