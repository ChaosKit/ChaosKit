add_library(ui_models
        Formula.h Formula.cpp
        Blend.cpp Blend.h
        System.cpp System.h
        toSource.cpp toSource.h)
set_target_properties(ui_models PROPERTIES AUTOMOC ON)
target_link_libraries(ui_models PUBLIC
        ast core library
        Qt5::Core
        Qt5::Gui)
