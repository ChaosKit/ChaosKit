#include "SimpleHistogramGenerator.h"
#include "ThreadLocalRng.h"
#include "toSource.h"

namespace chaoskit::core {

SimpleHistogramGenerator::SimpleHistogramGenerator(const SystemModel &system,
                                                   uint32_t width,
                                                   uint32_t height, std::shared_ptr<Rng> rng)
    : width_(width),
      height_(height),
      buffer_(width * height),
      iteration_count_(stdx::nullopt),
      interpreter_(toSource(system), Params::fromSystem(system)),
      rng_(std::move(rng)) {}

SimpleHistogramGenerator::SimpleHistogramGenerator(const SystemModel &system,
                                                   uint32_t width,
                                                   uint32_t height)
    : SimpleHistogramGenerator(system, width, height,
                               std::make_shared<ThreadLocalRng>()) {}

void SimpleHistogramGenerator::setSystem(const SystemModel &system) {
  interpreter_.setSystem(toSource(system));
  interpreter_.setParams(Params::fromSystem(system));
}

void SimpleHistogramGenerator::setSize(uint32_t width, uint32_t height) {
  width_ = width;
  height_ = height;
  buffer_.resize(width * height);
  clear();
}

void SimpleHistogramGenerator::setIterationCount(uint32_t count) {
  iteration_count_ = count;
}

void SimpleHistogramGenerator::setInfiniteIterationCount() {
  iteration_count_ = stdx::nullopt;
}

void SimpleHistogramGenerator::clear() {
  std::fill(buffer_.begin(), buffer_.end(), 0.f);
}

void SimpleHistogramGenerator::run() {
  Point point{rng_->randomFloat(-1.f, 1.f), rng_->randomFloat(-1.f, 1.f)};

  for (size_t i = 0; !iteration_count_ || i < *iteration_count_; i++) {
    auto [next_state, output] = interpreter_(point);
    point = next_state;
    add(output);
  }
}

void SimpleHistogramGenerator::add(const Point &point) {
  float x = (point.x() + 1.f) * (width_ * .5f);
  float y = (point.y() + 1.f) * (height_ * .5f);

  // Skip the point if out of bounds
  if (x < 0.f || x >= width_ || y < 0.f || y >= height_) {
    return;
  }

  add(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
}

void SimpleHistogramGenerator::add(uint32_t x, uint32_t y, float factor) {
  buffer_[y * width_ + x] += factor;
}

}  // namespace chaoskit::core
