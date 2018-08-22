#include "SimpleBlender.h"
#include "ThreadLocalRng.h"

namespace chaoskit {
namespace core {

SimpleBlender::SimpleBlender(const System &system, uint32_t width,
                             uint32_t height, std::shared_ptr<Rng> rng)
    : width_(width),
      height_(height),
      buffer_(width * height),
      iteration_count_(stdx::nullopt),
      interpreter_(system.toSource(), system.params()),
      rng_(std::move(rng)) {}

SimpleBlender::SimpleBlender(const System &system, uint32_t width,
                             uint32_t height)
    : SimpleBlender(system, width, height, std::make_shared<ThreadLocalRng>()) {
}

void SimpleBlender::setSystem(const System &system) {
  interpreter_.setSystem(system.toSource());
  interpreter_.setParams(system.params());
}

void SimpleBlender::setSize(uint32_t width, uint32_t height) {
  width_ = width;
  height_ = height;
  buffer_.resize(width * height);
  clear();
}

void SimpleBlender::setIterationCount(uint32_t count) {
  iteration_count_ = count;
}

void SimpleBlender::setInfiniteIterationCount() {
  iteration_count_ = stdx::nullopt;
}

void SimpleBlender::clear() { std::fill(buffer_.begin(), buffer_.end(), 0.f); }

void SimpleBlender::run() {
  Point point{rng_->randomFloat(-1.f, 1.f), rng_->randomFloat(-1.f, 1.f)};

  for (size_t i = 0; !iteration_count_ || i < *iteration_count_; i++) {
    auto[next_state, output] = interpreter_(point);
    point = next_state;
    add(output);
  }
}

void SimpleBlender::add(const Point &point) {
  float x = (point.x() + 1.f) * (width_ * .5f);
  float y = (point.y() + 1.f) * (height_ * .5f);

  // Skip the point if out of bounds
  if (x < 0.f || x >= width_ || y < 0.f || y >= height_) {
    return;
  }

  add(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
}

void SimpleBlender::add(uint32_t x, uint32_t y, float factor) {
  buffer_[y * width_ + x] += factor;
}

}  // namespace core
}  // namespace chaoskit
