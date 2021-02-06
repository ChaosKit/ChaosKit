#ifndef CHAOSKIT_UI_GATHERERTASK_H
#define CHAOSKIT_UI_GATHERERTASK_H

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSize>
#include <QTransform>
#include <QVector>
#include "ColorMap.h"
#include "HistogramBuffer.h"
#include "Point.h"
#include "core/Renderer.h"

namespace chaoskit::ui {

class GathererTask : public QObject {
  Q_OBJECT

 public:
  void synchronizeResult(core::Renderer *renderer);

 public slots:
  void addPoint(const chaoskit::core::Point &point, float color);
  void setSize(const QSize &size);
  void setColorMap(const chaoskit::core::ColorMap *colorMap);
  void clear();

 private:
  QTransform imageSpaceTransform_;
  QMutex mutex_;
  core::HistogramBuffer buffer_;
  bool isDirty_ = true;
  const core::ColorMap *colorMap_ = nullptr;

  void updateImageSpaceTransform(const QSizeF &size);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_GATHERERTASK_H
