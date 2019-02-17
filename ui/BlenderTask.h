#ifndef CHAOSKIT_UI_BLENDERTASK_H
#define CHAOSKIT_UI_BLENDERTASK_H

#include <core/SimpleInterpreter.h>
#include <QObject>
#include "Particle.h"
#include "models/System.h"

namespace chaoskit {
namespace ui {

class BlenderTask : public QObject {
  Q_OBJECT
 public:
  BlenderTask(std::shared_ptr<core::Rng> rng,
              int32_t ttl = core::Particle::IMMORTAL)
      : interpreter_(), particle_{}, ttl_(ttl), rng_(std::move(rng)) {}

 public slots:
  void setSystem(const chaoskit::ui::models::System *system);
  void start();
  void stop();
  void setTtl(int32_t ttl);

 signals:
  void started();
  void stopped();
  void stepCompleted(const chaoskit::core::Point &point, float color);

 private slots:
  void calculate();

 private:
  std::unique_ptr<core::SimpleInterpreter> interpreter_;
  core::Particle particle_;
  int32_t ttl_;
  bool running_ = false;
  std::shared_ptr<core::Rng> rng_;

  void randomizeParticle();
  void resetParticle();
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_BLENDERTASK_H
