add_library(ui_colormapregistry ColorMapRegistry.cpp ColorMapRegistry.h)
set_target_properties(ui_colormapregistry PROPERTIES AUTOMOC ON)
target_link_libraries(ui_colormapregistry PUBLIC core Qt5::Core)

add_library(ui_models BaseModel.h ColorMapModel.cpp ColorMapModel.h
                      ProjectModel.cpp ProjectModel.h)
set_target_properties(ui_models PROPERTIES AUTOMOC ON)
target_link_libraries(ui_models PUBLIC ui_storage_proto Qt5::Core)
