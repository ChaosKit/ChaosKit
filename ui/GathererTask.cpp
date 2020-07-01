#include "GathererTask.h"
#include <QRectF>

namespace chaoskit::ui {

using core::Color;
using core::HistogramBuffer;
using core::Point;

void GathererTask::addPoint(const Point &point, float color) {
  // Calculate coordinates from the unit rect to the buffer coordinates
  //  int x = static_cast<int>((point.x() + 1.f) * .5f * buffer_.width());
  //  int y = static_cast<int>((point.y() + 1.f) * .5f * buffer_.height());
  auto p = imageSpaceTransform_.map(QPointF(point.x(), point.y()));
  int x = static_cast<int>(p.x());
  int y = static_cast<int>(p.y());

  // Add the color if it fits inside
  if (x >= 0 && y >= 0 && x < buffer_.width() && y < buffer_.height()) {
    auto *entry = buffer_(static_cast<size_t>(x), static_cast<size_t>(y));
    Color mappedColor{1, 1, 1, color};
    if (colorMap_) {
      mappedColor = colorMap_->map(color);
    }

    mutex_.lock();
    *entry += mappedColor;
    mutex_.unlock();
  }
}

void GathererTask::synchronizeResult(core::Renderer *renderer) {
  QMutexLocker locker(&mutex_);
  renderer->updateHistogramBuffer(buffer_);
}

void GathererTask::setSize(const QSize &size) {
  QMutexLocker locker(&mutex_);
  buffer_.resize(static_cast<size_t>(size.width()),
                 static_cast<size_t>(size.height()));
  updateImageSpaceTransform(size);
}

void GathererTask::setColorMap(const chaoskit::core::ColorMap *colorMap) {
  QMutexLocker locker(&mutex_);
  colorMap_ = colorMap;
  buffer_.clear();
}

void GathererTask::clear() {
  QMutexLocker locker(&mutex_);
  buffer_.clear();
}

void GathererTask::updateImageSpaceTransform(const QSizeF &size) {
  static QRectF sourceBounds(-1, -1, 2, 2);

  imageSpaceTransform_ =
      QTransform::fromScale(size.width() / sourceBounds.width(),
                            size.height() / sourceBounds.height())
          .translate(-sourceBounds.x(), -sourceBounds.y());

  // Zoom to fill
  if (size.width() < size.height()) {
    imageSpaceTransform_.scale(size.height() / size.width(), 1);
  } else {
    imageSpaceTransform_.scale(1, size.width() / size.height());
  }
}

}  // namespace chaoskit::ui
