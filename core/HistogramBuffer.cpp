#include "HistogramBuffer.h"
#include <algorithm>

namespace chaoskit::core {

void HistogramBuffer::clear() {
  std::fill(buffer_.begin(), buffer_.end(), Color::zero());
}

void HistogramBuffer::resize(size_t width, size_t height) {
  buffer_.assign(width * height, Color::zero());
  width_ = width;
  height_ = height;
}

}  // namespace chaoskit::core
