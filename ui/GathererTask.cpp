#include "GathererTask.h"
#include <QColor>

namespace chaoskit::ui {

using core::HistogramBuffer;
using core::HistogramColor;
using core::Point;

void GathererTask::addPoint(const Point &point, float color) {
  // Calculate coordinates from the unit rect to the buffer coordinates
  int x = static_cast<int>((point.x() + 1.f) * .5f * buffer_.width());
  int y = static_cast<int>((point.y() + 1.f) * .5f * buffer_.height());

  // Add the color if it fits inside
  if (x >= 0 && y >= 0 && x < buffer_.width() && y < buffer_.height()) {
    auto *entry = buffer_(static_cast<size_t>(x), static_cast<size_t>(y));
    // TODO: replace this with a palette
    auto mappedColor = QColor::fromHsvF(color, 0.7, 0.9);

    mutex_.lock();
    *entry += HistogramColor{static_cast<float>(mappedColor.redF()),
                             static_cast<float>(mappedColor.greenF()),
                             static_cast<float>(mappedColor.blueF()), 1.f};
    mutex_.unlock();
  }
}

void GathererTask::setSize(const QSize &size) {
  QMutexLocker locker(&mutex_);
  buffer_.resize(static_cast<size_t>(size.width()),
                 static_cast<size_t>(size.height()));
}

void GathererTask::clear() {
  QMutexLocker locker(&mutex_);
  buffer_.clear();
}

void GathererTask::withHistogram(
    const std::function<void(const HistogramBuffer &)> &action) {
  QMutexLocker locker(&mutex_);
  action(buffer_);
}

}  // namespace chaoskit::ui
