#ifndef CHAOSKIT_UI_HISTOGRAMGENERATOR_H
#define CHAOSKIT_UI_HISTOGRAMGENERATOR_H

#include <QObject>
#include <QThread>
#include "BlenderTask.h"
#include "GathererTask.h"
#include "HistogramBuffer.h"
#include "models/System.h"

namespace chaoskit {
namespace ui {

class HistogramGenerator : public QObject {
  Q_OBJECT
 public:
  HistogramGenerator(QObject *parent = nullptr);
  ~HistogramGenerator();

  void withHistogram(
      const std::function<void(const core::HistogramBuffer &)> &action);

  bool running() const { return running_; }

 public slots:
  void setSystem(const chaoskit::ui::System *system);
  void setSize(quint32 width, quint32 height);
  void setTtl(int32_t ttl);
  void start();
  void stop();
  void clear();

 signals:
  void started();
  void stopped();

 private:
  QThread *thread_;
  BlenderTask *blenderTask_;
  GathererTask *gathererTask_;
  bool running_ = false;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_HISTOGRAMGENERATOR_H
