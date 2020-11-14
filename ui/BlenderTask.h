#ifndef CHAOSKIT_UI_BLENDERTASK_H
#define CHAOSKIT_UI_BLENDERTASK_H

#include <core/SimpleInterpreter.h>
#include <QObject>
#include "Particle.h"

namespace chaoskit::ui {

class BlenderTask : public QObject {
  Q_OBJECT
 public:
  explicit BlenderTask(std::shared_ptr<core::Rng> rng)
      : interpreter_(), particle_{}, rng_(std::move(rng)) {}

 public slots:
  void setSystem(const chaoskit::flame::System *system);
  void start();
  void stop();

 signals:
  void started();
  void stopped();
  void stepCompleted(const chaoskit::core::Point &point, float color);

 private slots:
  void calculate();

 private:
  std::unique_ptr<core::SimpleInterpreter> interpreter_;
  core::Particle particle_;
  bool running_ = false;
  std::shared_ptr<core::Rng> rng_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_BLENDERTASK_H
