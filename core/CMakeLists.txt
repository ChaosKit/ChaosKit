add_library(
  core
  BlackWhiteColorMap.h
  CameraSystem.h
  Color.h
  ColorMap.h
  ColorMapRegistry.cpp
  ColorMapRegistry.h
  debug.cpp
  debug.h
  errors.cpp
  errors.h
  ExpressionInterpreter.cpp
  ExpressionInterpreter.h
  Generator.h
  HistogramBuffer.cpp
  HistogramBuffer.h
  PaletteColorMap.cpp
  PaletteColorMap.h
  Particle.h
  Point.h
  RainbowColorMap.cpp
  RainbowColorMap.h
  random.h
  Renderer.h
  Rng.h
  SimpleHistogramGenerator.cpp
  SimpleHistogramGenerator.h
  SimpleInterpreter.cpp
  SimpleInterpreter.h
  SystemParticle.h
  SystemProcessor.cpp
  SystemProcessor.h
  ThreadLocalRng.cpp
  ThreadLocalRng.h
  TransformIndex.cpp
  TransformIndex.h
  TransformInterpreter.cpp
  TransformInterpreter.h
  TransformParams.h
  TransformSystem.h)
target_link_libraries(
  core
  # TODO: remove flame after interpreter migration
  PUBLIC ast flame stdx randutils
  PRIVATE magic_enum xxHash::xxhash
  INTERFACE flame_structures)

add_executable(
  core_test
  ExpressionInterpreterTest.cpp
  SimpleInterpreterTest.cpp
  SystemProcessorTest.cpp
  testing/MockRng.h
  testing/StaticRng.h
  TransformIndexTest.cpp
  TransformInterpreterTest.cpp)

target_link_libraries(core_test PRIVATE gmock gmock_main ast core)
add_test(NAME core_test COMMAND core_test)
