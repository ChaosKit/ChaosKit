#include "BlenderTask.h"
#include <QDebug>
#include <QTimer>
#include "core/errors.h"
#include "flame/toSystem.h"

using chaoskit::core::CameraSystemProcessor;
using chaoskit::core::MissingParameterError;
using chaoskit::core::OutputNotAvailable;
using chaoskit::core::UndefinedVariableError;
using chaoskit::flame::toCameraSystem;
using chaoskit::flame::Transform;

namespace chaoskit::ui {

void BlenderTask::setSystem(const core::CameraSystem *system) {
  processor_ = std::make_unique<CameraSystemProcessor>(*system, rng_);
  particle_ = processor_->createParticle();
}

void BlenderTask::start() {
  if (!processor_ || running_) {
    return;
  }

  running_ = true;
  emit started();
  QTimer::singleShot(0, this, &BlenderTask::calculate);
}

void BlenderTask::stop() {
  if (!running_) {
    return;
  }

  running_ = false;
  emit stopped();
}

void BlenderTask::calculate() {
  if (!running_) {
    return;
  }

  try {
    particle_ = processor_->process(particle_);
    auto output = processor_->processCamera(particle_);
    emit stepCompleted(output.point, output.color);

    QTimer::singleShot(0, this, &BlenderTask::calculate);
  } catch (MissingParameterError &e) {
    qCritical() << "In BlenderTask::calculate():" << e.what();
    stop();
  } catch (OutputNotAvailable &e) {
    qCritical() << "In BlenderTask::calculate():" << e.what();
    stop();
  } catch (UndefinedVariableError &e) {
    qCritical() << "In BlenderTask::calculate():" << e.what();
    stop();
  }
}

}  // namespace chaoskit::ui
