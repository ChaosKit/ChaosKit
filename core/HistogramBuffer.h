#ifndef CHAOSKIT_CORE_HISTOGRAMBUFFER_H
#define CHAOSKIT_CORE_HISTOGRAMBUFFER_H

#include <cstddef>
#include <vector>
#include "Color.h"

namespace chaoskit::core {

class HistogramBuffer {
 public:
  HistogramBuffer() : HistogramBuffer(0, 0) {}
  HistogramBuffer(size_t width, size_t height)
      : width_(width), height_(height), buffer_(width * height) {}

  Color* operator()(size_t x, size_t y) { return &buffer_[index(x, y)]; }

  [[nodiscard]] size_t width() const { return width_; }
  [[nodiscard]] size_t height() const { return height_; }
  [[nodiscard]] size_t size() const { return buffer_.size(); }
  void clear();
  void resize(size_t width, size_t height);

  Color* data() { return buffer_.data(); }
  [[nodiscard]] const Color* data() const { return buffer_.data(); }

 private:
  [[nodiscard]] size_t index(size_t x, size_t y) const {
    return y * width_ + x;
  }

  size_t width_, height_;
  std::vector<Color> buffer_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_HISTOGRAMBUFFER_H
