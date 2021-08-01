#ifndef CHAOSKIT_UI_BLENDERTASK_H
#define CHAOSKIT_UI_BLENDERTASK_H

#include <QObject>
#include "Particle.h"
#include "core/CameraSystemProcessor.h"

namespace chaoskit::ui {

class BlenderTask : public QObject {
  Q_OBJECT
 public:
  explicit BlenderTask(std::shared_ptr<core::Rng> rng)
      : processor_(), particle_{}, rng_(std::move(rng)) {}

 public slots:
  void setSystem(const chaoskit::core::CameraSystem *system);
  void setLifetimeRange(int32_t min, int32_t max);
  void start();
  void stop();

 signals:
  void started();
  void stopped();
  void stepCompleted(const chaoskit::core::Point &point, float color);

 private slots:
  void calculate();

 private:
  std::unique_ptr<core::CameraSystemProcessor> processor_;
  core::SystemParticle particle_;
  bool running_ = false;
  std::shared_ptr<core::Rng> rng_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_BLENDERTASK_H
