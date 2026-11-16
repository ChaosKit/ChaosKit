#ifndef CHAOSKIT_UI_RHICOMPUTEENGINE_H
#define CHAOSKIT_UI_RHICOMPUTEENGINE_H

#include <rhi/qrhi.h>
#include <rhi/qshader.h>
#include <QByteArray>
#include <QSize>
#include <QString>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "core/CameraSystem.h"
#include "core/ColorMap.h"
#include "core/GpuSystemCompiler.h"

namespace chaoskit::ui {

// Manages GPU-resident compute resources (ParticleSSBO, HistogramSSBO,
// CounterSSBO, 1024x1 RGBA32F ColorMapLUT), caches compiled QShader +
// QRhiComputePipeline entries by AST structural key, and records GPU
// accumulation dispatches onto QRhiCommandBuffer.
class RhiComputeEngine {
 public:
  static constexpr uint32_t kDefaultParticleCount = 16384;
  static constexpr uint32_t kColorMapLutWidth = 1024;

  RhiComputeEngine() = default;
  ~RhiComputeEngine();

  // Non-copyable and non-movable: owns GPU buffers, textures, and pipeline
  // handles.
  RhiComputeEngine(const RhiComputeEngine&) = delete;
  RhiComputeEngine& operator=(const RhiComputeEngine&) = delete;
  RhiComputeEngine(RhiComputeEngine&&) = delete;
  RhiComputeEngine& operator=(RhiComputeEngine&&) = delete;

  bool initialize(QRhi* rhi, QString* errorMessage = nullptr);
  void releaseResources();

  bool setSystem(const core::CameraSystem& system,
                 QString* errorMessage = nullptr);
  void clearSystem();
  void setColorMap(const core::ColorMap* colorMap);
  void setSize(const QSize& size);
  void setParticleCount(uint32_t particleCount);
  void setSeed(std::optional<uint64_t> seed);
  void reset();

  // Ensures shared buffers exist, applies any pending ColorMap LUT upload, and
  // executes clear/init passes if needsReset_ is set, without stepping
  // particles. Works even before a System is assigned (zeros
  // Histogram/Counter).
  bool prepareAndClear(QRhiCommandBuffer* cb, QString* errorMessage = nullptr);

  // Records any pending clear/init/color-map updates and dispatches one
  // particle accumulation step with `ticksPerInvocation` ticks per particle.
  bool dispatch(QRhiCommandBuffer* cb, uint32_t ticksPerInvocation,
                QString* errorMessage = nullptr);

  [[nodiscard]] QRhiBuffer* histogramBuffer() const {
    return histogramBuffer_.get();
  }
  [[nodiscard]] QRhiBuffer* counterBuffer() const {
    return counterBuffer_.get();
  }
  [[nodiscard]] QSize size() const { return size_; }
  [[nodiscard]] uint32_t particleCount() const { return particleCount_; }
  [[nodiscard]] uint64_t resourceEpoch() const { return resourceEpoch_; }
  [[nodiscard]] size_t cachedPipelineCount() const {
    return pipelineCache_.size();
  }
  [[nodiscard]] uint64_t compileCount() const { return compileCount_; }

  [[nodiscard]] static QByteArray clearShaderSource();
  [[nodiscard]] static QByteArray initShaderSource();
  [[nodiscard]] static uint64_t decodePlottedCounter(
      const QByteArray& counterData);

 private:
  struct CachedPipeline {
    QShader shader;
    std::unique_ptr<QRhiComputePipeline> pipeline;
    std::unique_ptr<QRhiShaderResourceBindings> srb;
    core::UniformLayout layout;
    uint64_t boundEpoch = 0;
  };

  bool ensureBuiltinPipelines(QString* errorMessage);
  bool ensureSharedResources(uint32_t requiredUniformSize,
                             QString* errorMessage);
  bool ensurePipelineBindings(CachedPipeline& entry, QString* errorMessage);
  bool prepareUpdatesAndReset(QRhiCommandBuffer* cb, CachedPipeline* cached,
                              uint32_t ticksPerInvocation,
                              QRhiResourceUpdateBatch*& updatesOut,
                              QString* errorMessage);
  void refreshActiveSeed();

  QRhi* rhi_ = nullptr;
  QShader clearShader_;
  QShader initShader_;
  std::unique_ptr<QRhiComputePipeline> clearPipeline_;
  std::unique_ptr<QRhiComputePipeline> initPipeline_;
  std::unique_ptr<QRhiShaderResourceBindings> clearSrb_;
  std::unique_ptr<QRhiShaderResourceBindings> initSrb_;

  std::unique_ptr<QRhiBuffer> clearUniformBuffer_;
  std::unique_ptr<QRhiBuffer> uniformBuffer_;
  std::unique_ptr<QRhiBuffer> particleBuffer_;
  std::unique_ptr<QRhiBuffer> histogramBuffer_;
  std::unique_ptr<QRhiBuffer> counterBuffer_;
  std::unique_ptr<QRhiTexture> colorMapTexture_;
  std::unique_ptr<QRhiSampler> colorMapSampler_;

  std::unordered_map<std::string, CachedPipeline> pipelineCache_;
  std::optional<core::CameraSystem> system_ = std::nullopt;
  std::string activeStructuralKey_;

  const core::ColorMap* colorMap_ = nullptr;
  QSize size_ = QSize(1, 1);
  uint32_t particleCount_ = kDefaultParticleCount;
  bool seededMode_ = false;
  uint64_t configuredSeed_ = 0;
  uint64_t activeSeed_ = 0;
  uint64_t tickOffset_ = 0;

  bool needsReset_ = true;
  bool colorMapDirty_ = true;
  bool particlesDirty_ = true;
  bool histogramDirty_ = true;
  uint64_t resourceEpoch_ = 0;
  uint64_t clearBoundEpoch_ = 0;
  uint64_t initBoundEpoch_ = 0;
  uint64_t compileCount_ = 0;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_RHICOMPUTEENGINE_H
