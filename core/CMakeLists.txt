add_library(
  core
  BlackWhiteColorMap.h
  CameraSystemProcessor.cpp
  CameraSystemProcessor.h
  ColorMapRegistry.cpp
  ColorMapRegistry.h
  debug.cpp
  debug.h
  errors.cpp
  errors.h
  ExpressionInterpreter.cpp
  ExpressionInterpreter.h
  HistogramBuffer.cpp
  HistogramBuffer.h
  PaletteColorMap.cpp
  PaletteColorMap.h
  RainbowColorMap.cpp
  RainbowColorMap.h
  SimpleHistogramGenerator.cpp
  SimpleHistogramGenerator.h
  SystemProcessor.cpp
  SystemProcessor.h
  ThreadLocalRng.cpp
  ThreadLocalRng.h
  TransformInterpreter.cpp
  TransformInterpreter.h)
target_link_libraries(
  core
  PUBLIC ast core_interface stdx
  PRIVATE magic_enum pcg_random)

add_library(
  core_interface
  CameraSystem.h
  Color.h
  ColorMap.h
  Generator.h
  Particle.h
  Point.h
  Renderer.h
  Rng.h
  SystemParticle.h
  TransformIndex.cpp
  TransformIndex.h
  TransformParams.h
  TransformSystem.h)
target_link_libraries(
  core_interface
  INTERFACE ast
  PRIVATE xxHash::xxhash)

add_executable(
  core_test
  CameraSystemProcessorTest.cpp
  ExpressionInterpreterTest.cpp
  SystemProcessorTest.cpp
  testing/MockRng.h
  testing/StaticRng.h
  TransformIndexTest.cpp
  TransformInterpreterTest.cpp)

target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
