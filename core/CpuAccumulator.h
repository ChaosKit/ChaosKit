#ifndef CHAOSKIT_CORE_CPUACCUMULATOR_H
#define CHAOSKIT_CORE_CPUACCUMULATOR_H

#include <cstdint>
#include <memory>
#include "CameraSystem.h"
#include "CameraSystemProcessor.h"
#include "ColorMap.h"
#include "HistogramBuffer.h"
#include "Rng.h"
#include "SystemParticle.h"

namespace chaoskit::core {

// Executes batched CPU particle accumulation into a 2D HistogramBuffer while
// tracking exact plotted iteration counts (excluding warm-up and discarded
// non-finite steps).
class CpuAccumulator {
 public:
  CpuAccumulator(const CameraSystem& system, uint32_t width, uint32_t height,
                 const ColorMap* colorMap = nullptr,
                 std::shared_ptr<Rng> rng = nullptr);

  void setSystem(const CameraSystem& system);
  void setColorMap(const ColorMap* colorMap);
  void setSize(uint32_t width, uint32_t height);
  void reset();

  // Executes `stepCount` simulation ticks and returns the number of finite
  // non-warmup iterations plotted during this batch.
  uint64_t accumulate(uint64_t stepCount);

  // Executes simulation ticks until `targetPlotted` finite non-warmup
  // iterations are plotted (or a safety limit of consecutive unplotted ticks
  // is reached), returning the actual plotted count added by this call.
  uint64_t accumulatePlotted(uint64_t targetPlotted);

  // Sums another accumulator's histogram and plotted iterations into this
  // accumulator. Dimensions must match.
  void merge(const CpuAccumulator& other);

  // Computes the maximum consecutive zero-progress ticks before bailing out of
  // bounded-plotted accumulation for a given `minLifetime`.
  [[nodiscard]] static uint64_t maxConsecutiveZeroTicks(
      int minLifetime) noexcept;

  [[nodiscard]] const HistogramBuffer& histogram() const { return buffer_; }
  [[nodiscard]] uint64_t plottedIterations() const {
    return plottedIterations_;
  }
  [[nodiscard]] uint32_t width() const {
    return static_cast<uint32_t>(buffer_.width());
  }
  [[nodiscard]] uint32_t height() const {
    return static_cast<uint32_t>(buffer_.height());
  }
  [[nodiscard]] double iterationsPerPixel() const;

 private:
  CameraSystemProcessor processor_;
  SystemParticle particle_;
  const ColorMap* colorMap_ = nullptr;
  HistogramBuffer buffer_;
  uint64_t plottedIterations_ = 0;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_CPUACCUMULATOR_H
