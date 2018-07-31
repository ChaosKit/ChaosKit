#ifndef CHAOSKIT_CORE_HISTOGRAMBUFFER_H
#define CHAOSKIT_CORE_HISTOGRAMBUFFER_H

#include <cstddef>
#include <vector>
#include "HistogramColor.h"

namespace chaoskit {
namespace core {

class HistogramBuffer {
 public:
  HistogramBuffer(size_t width, size_t height)
      : width_(width), height_(height), buffer_(width * height) {}

  HistogramColor* operator()(size_t x, size_t y) {
    return &buffer_[index(x, y)];
  }

  size_t width() const { return width_; }
  size_t height() const { return height_; }
  size_t size() const { return buffer_.size(); }

  HistogramColor* data() { return buffer_.data(); }
  const HistogramColor* data() const { return buffer_.data(); }

 private:
  size_t index(size_t x, size_t y) const { return y * width_ + x; }

  size_t width_, height_;
  std::vector<HistogramColor> buffer_;
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_HISTOGRAMBUFFER_H
