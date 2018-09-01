#include "HistogramBuffer.h"

namespace chaoskit {
namespace core {

void HistogramBuffer::clear() {
  std::fill(buffer_.begin(), buffer_.end(), HistogramColor::zero());
}

void HistogramBuffer::resize(size_t width, size_t height) {
  clear();
  buffer_.resize(width * height);
  width_ = width;
  height_ = height;
}

}
}
