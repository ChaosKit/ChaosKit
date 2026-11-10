add_library(ui_colormapregistry ColorMapRegistry.cpp ColorMapRegistry.h)
set_target_properties(ui_colormapregistry PROPERTIES AUTOMOC ON)
target_link_libraries(ui_colormapregistry PUBLIC core Qt::Core)

add_library(
  ui_models
  BaseModel.h
  BlendModel.cpp
  BlendModel.h
  ColoringMethodModel.cpp
  ColoringMethodModel.h
  ColorMapModel.cpp
  ColorMapModel.h
  FormulaModel.cpp
  FormulaModel.h
  ModelFactory.cpp
  ModelFactory.h
  ProjectModel.cpp
  ProjectModel.h
  SystemModel.cpp
  SystemModel.h
  TransformModel.cpp
  TransformModel.h)
set_target_properties(ui_models PROPERTIES AUTOMOC ON)
target_link_libraries(
  ui_models
  PUBLIC core
         library
         ui_colormapregistry
         ui_storage
         Qt::Core
         Qt::Gui
         qt_qml_models
  PRIVATE magic_enum)

add_executable(ui_models_test SystemModelTest.cpp)
set_target_properties(ui_models_test PROPERTIES AUTOMOC ON)
target_link_libraries(ui_models_test PRIVATE gmock gmock_main ui_models
                                             Qt::Core Qt::Test)
add_test(NAME ui_models_test COMMAND ui_models_test)
