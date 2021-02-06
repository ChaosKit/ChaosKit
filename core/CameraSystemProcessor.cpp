#include "CameraSystemProcessor.h"

namespace chaoskit::core {

void CameraSystemProcessor::setCameraSystem(const CameraSystem& cameraSystem) {
  setSystem(cameraSystem.system);
  setCamera(cameraSystem.camera);
}

SystemParticle CameraSystemProcessor::processCamera(
    SystemParticle input) const {
  if (!camera_) return input;

  SystemParticle output;
  output.applyParticle(interpreter_.interpret(
      input.asParticle(), camera_->transform, camera_->params));
  return output;
}

}  // namespace chaoskit::core
