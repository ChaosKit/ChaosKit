#ifndef CHAOSKIT_CORE_GPUSYSTEMCOMPILER_H
#define CHAOSKIT_CORE_GPUSYSTEMCOMPILER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "CameraSystem.h"
#include "TransformIndex.h"

namespace chaoskit::core {

struct GpuDispatchConfig {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t particleCount = 0;
  uint32_t ticksPerInvocation = 1;
  bool seededMode = false;
  uint64_t seed = 0;
  uint64_t tickOffset = 0;
};

struct UniformLayout {
  struct AffineSlot {
    TransformIndex index;
    uint32_t floatOffset = 0;
    bool isCamera = false;
  };

  struct ParamSlot {
    TransformIndex index;
    size_t paramIndex = 0;
    uint32_t floatOffset = 0;
    bool isCamera = false;
  };

  struct WeightedSlot {
    TransformIndex index;
    size_t branchCount = 0;
    uint32_t floatOffset = 0;
    bool isCamera = false;
  };

  std::string structuralKey;
  uint32_t packedFloatCount = 0;
  uint32_t packedVec4Count = 1;
  uint32_t uniformBufferSize = 80;
  std::vector<AffineSlot> affineSlots;
  std::vector<ParamSlot> paramSlots;
  std::vector<WeightedSlot> randomChoiceSlots;
  std::vector<WeightedSlot> weightedSumSlots;
};

struct CompiledGpuSystem {
  std::string structuralKey;
  std::string computeShaderSource;
  UniformLayout layout;
};

// Compiles a Qt-free CameraSystem into a Vulkan-flavored GLSL (#version 450)
// compute shader, a deterministic structural cache key, and a std140 uniform
// layout/packer that keeps continuous parameters out of shader recompilation.
class GpuSystemCompiler {
 public:
  static constexpr uint32_t kWorkgroupSize = 64;
  static constexpr uint32_t kHeaderByteSize = 64;

  [[nodiscard]] static std::string computeStructuralKey(
      const CameraSystem& cameraSystem);

  [[nodiscard]] static CompiledGpuSystem compile(
      const CameraSystem& cameraSystem);

  [[nodiscard]] static std::vector<uint8_t> packUniforms(
      const UniformLayout& layout, const CameraSystem& cameraSystem,
      const GpuDispatchConfig& config);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_GPUSYSTEMCOMPILER_H
