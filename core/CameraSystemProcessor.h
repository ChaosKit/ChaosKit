#ifndef CHAOSKIT_CORE_CAMERASYSTEMPROCESSOR_H
#define CHAOSKIT_CORE_CAMERASYSTEMPROCESSOR_H

#include "CameraSystem.h"
#include "SystemProcessor.h"

#include <utility>

namespace chaoskit::core {

class CameraSystemProcessor : public SystemProcessor {
 public:
  CameraSystemProcessor(TransformSystem system, std::shared_ptr<Rng> rng)
      : SystemProcessor(std::move(system), std::move(rng)), camera_() {}
  CameraSystemProcessor(const CameraSystem& cameraSystem,
                        std::shared_ptr<Rng> rng)
      : SystemProcessor(cameraSystem.system, std::move(rng)),
        camera_(cameraSystem.camera) {}

  void setCamera(stdx::optional<TransformSystem> camera) {
    camera_ = std::move(camera);
  }
  void setCameraSystem(const CameraSystem& cameraSystem);

  [[nodiscard]] SystemParticle processCamera(SystemParticle input) const;

 private:
  stdx::optional<TransformSystem> camera_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_CAMERASYSTEMPROCESSOR_H
