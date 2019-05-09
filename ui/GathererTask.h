#ifndef CHAOSKIT_UI_GATHERERTASK_H
#define CHAOSKIT_UI_GATHERERTASK_H

#include <QObject>
#include <QSize>
#include <QVector>
#include "HistogramBuffer.h"
#include "Point.h"

namespace chaoskit::ui {

class GathererTask : public QObject {
  Q_OBJECT

 public:
  void withHistogram(
      const std::function<void(const core::HistogramBuffer &)> &action);

 public slots:
  void addPoint(const chaoskit::core::Point &point, float color);
  void setSize(const QSize &size);
  void clear();

 private:
  QMutex mutex_;
  core::HistogramBuffer buffer_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_GATHERERTASK_H
