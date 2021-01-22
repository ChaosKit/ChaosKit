#ifndef CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H
#define CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H

#include <stdx/optional.h>
#include <vector>

#include "Color.h"
#include "ColorMap.h"
#include "Generator.h"
#include "SystemProcessor.h"
#include "flame/System.h"

namespace chaoskit::core {

class SimpleHistogramGenerator : public Generator {
 public:
  SimpleHistogramGenerator(const flame::System &system, uint32_t width,
                           uint32_t height, std::shared_ptr<Rng> rng);
  SimpleHistogramGenerator(const flame::System &system, uint32_t width,
                           uint32_t height);

  void setSystem(const flame::System &system) override;
  void setSize(uint32_t width, uint32_t height);
  void setColorMap(const ColorMap *color_map);
  void setIterationCount(uint32_t count);

  void synchronizeResult(Renderer *renderer) override;
  void reset() override;

  [[nodiscard]] const Color *data() const { return buffer_.data(); }

 private:
  HistogramBuffer buffer_;
  stdx::optional<uint32_t> iteration_count_;
  SystemProcessor system_processor_;
  TransformInterpreter camera_interpreter_;
  stdx::optional<TransformSystem> camera_system_;
  const ColorMap *color_map_;

  void setCameraSystem(const flame::System &system);

  void add(const Particle &particle);
  void add(uint32_t x, uint32_t y, float factor = 1);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H
