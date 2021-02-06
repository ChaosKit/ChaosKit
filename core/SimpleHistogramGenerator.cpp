#include "SimpleHistogramGenerator.h"

#include <utility>
#include "ThreadLocalRng.h"

namespace chaoskit::core {

SimpleHistogramGenerator::SimpleHistogramGenerator(const CameraSystem &system,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   std::shared_ptr<Rng> rng)
    : buffer_(width, height),
      iteration_count_(stdx::nullopt),
      system_processor_(system, std::move(rng)),
      color_map_(nullptr) {}

SimpleHistogramGenerator::SimpleHistogramGenerator(const CameraSystem &system,
                                                   uint32_t width,
                                                   uint32_t height)
    : SimpleHistogramGenerator(system, width, height,
                               std::make_shared<ThreadLocalRng>()) {}

void SimpleHistogramGenerator::setSystem(const CameraSystem &system) {
  system_processor_.setCameraSystem(system);
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

void SimpleHistogramGenerator::setLifetimeRange(int32_t min, int32_t max) {
  system_processor_.setSkip(min);
  system_processor_.setParticleLifetime(max);
}

void SimpleHistogramGenerator::reset() { buffer_.clear(); }

void SimpleHistogramGenerator::synchronizeResult(Renderer *renderer) {
  auto particle = system_processor_.createParticle();

  for (size_t i = 0; !iteration_count_ || i < *iteration_count_; i++) {
    if (!enabled_) break;
    particle = system_processor_.process(particle);
    add(system_processor_.processCamera(particle));
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
