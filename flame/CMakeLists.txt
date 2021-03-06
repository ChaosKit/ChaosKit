add_library(flame_structures INTERFACE)
target_sources(
  flame_structures
  INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Blend.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/ColoringMethod.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/Document.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/Formula.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/System.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/Transform.h")
target_link_libraries(flame_structures INTERFACE ast library)

add_library(flame_hierarchy INTERFACE)
target_sources(flame_hierarchy
               INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/hierarchy.h")
target_link_libraries(flame_hierarchy INTERFACE flame_structures state)

add_library(
  flame
  DocumentDeleter.cpp
  DocumentDeleter.h
  DocumentVisitor.h
  ManagedDocument.cpp
  ManagedDocument.h
  SystemIndex.h
  toParams.cpp
  toParams.h
  toSystem.cpp
  toSystem.h
  toTransform.cpp
  toTransform.h
  transforms.cpp
  transforms.h
  util.cpp
  util.h)
target_link_libraries(
  flame
  PUBLIC ast core_interface
  INTERFACE flame_structures
  PRIVATE magic_enum)
