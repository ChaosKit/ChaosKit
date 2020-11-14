#ifndef CHAOSKIT_UI_HISTOGRAMGENERATOR_H
#define CHAOSKIT_UI_HISTOGRAMGENERATOR_H

#include <QObject>
#include <QThread>
#include "BlenderTask.h"
#include "GathererTask.h"
#include "HistogramBuffer.h"
#include "core/Generator.h"
#include "flame/System.h"

namespace chaoskit::ui {

class HistogramGenerator : public QObject, public core::Generator {
  Q_OBJECT
 public:
  explicit HistogramGenerator(QObject *parent = nullptr);
  ~HistogramGenerator() override;

  void synchronizeResult(core::Renderer *renderer) override;
  void setEnabled(bool enabled) override;
  void setSystem(const flame::System &system) override;

 public slots:
  void setColorMap(const chaoskit::core::ColorMap *colorMap);
  void setSize(quint32 width, quint32 height);
  void start();
  void stop();
  void reset() override;

 signals:
  void started();
  void stopped();

 private:
  QThread *thread_;
  BlenderTask *blenderTask_;
  GathererTask *gathererTask_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_HISTOGRAMGENERATOR_H
