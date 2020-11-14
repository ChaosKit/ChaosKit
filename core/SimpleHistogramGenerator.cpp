#include "SimpleHistogramGenerator.h"
#include "ThreadLocalRng.h"
#include "flame/Params.h"
#include "flame/toSource.h"

namespace chaoskit::core {

SimpleHistogramGenerator::SimpleHistogramGenerator(const flame::System &system,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   std::shared_ptr<Rng> rng)
    : buffer_(width, height),
      iteration_count_(stdx::nullopt),
      interpreter_(system, std::move(rng)),
      color_map_(nullptr) {}

SimpleHistogramGenerator::SimpleHistogramGenerator(const flame::System &system,
                                                   uint32_t width,
                                                   uint32_t height)
    : SimpleHistogramGenerator(system, width, height,
                               std::make_shared<ThreadLocalRng>()) {}

void SimpleHistogramGenerator::setSystem(const flame::System &system) {
  interpreter_.setSystem(toSource(system));
  interpreter_.setParams(flame::Params::fromSystem(system));
}

void SimpleHistogramGenerator::setSize(uint32_t width, uint32_t height) {
  buffer_.resize(width, height);
}

void SimpleHistogramGenerator::setIterationCount(uint32_t count) {
  iteration_count_ = count;
}

void SimpleHistogramGenerator::setColorMap(const ColorMap *color_map) {
  color_map_ = color_map;
}

void SimpleHistogramGenerator::reset() { buffer_.clear(); }

void SimpleHistogramGenerator::synchronizeResult(Renderer *renderer) {
  auto particle = interpreter_.randomizeParticle();

  for (size_t i = 0; !iteration_count_ || i < *iteration_count_; i++) {
    if (!enabled_) break;
    auto [next_state, output] = interpreter_(particle);
    particle = next_state;
    add(output);
  }

  renderer->updateHistogramBuffer(buffer_);
}
void SimpleHistogramGenerator::add(const Particle &particle) {
  float x = (particle.x() + 1.f) * (buffer_.width() * .5f);
  float y = (particle.y() + 1.f) * (buffer_.height() * .5f);

  // Skip the point if out of bounds
  if (x < 0.f || x >= buffer_.width() || y < 0.f || y >= buffer_.height()) {
    return;
  }

  add(static_cast<uint32_t>(x), static_cast<uint32_t>(y), particle.color);
}

void SimpleHistogramGenerator::add(uint32_t x, uint32_t y, float factor) {
  Color *color = buffer_(x, y);

  if (color_map_) {
    *color += color_map_->map(factor);
  } else {
    *color += {1, 1, 1, factor};
  }
}

}  // namespace chaoskit::core
