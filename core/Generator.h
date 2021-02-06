#ifndef CHAOSKIT_CORE_GENERATOR_H
#define CHAOSKIT_CORE_GENERATOR_H

#include "CameraSystem.h"
#include "Renderer.h"

namespace chaoskit::core {

class Generator {
 public:
  /**
   * Synchronizes the current result with the renderer.
   * Usually, this is called by the render loop just before rendering the
   * system.
   */
  virtual void synchronizeResult(Renderer* renderer) = 0;

  /**
   * Finalizes the current generation step.
   * Usually, this is called by the render loop just after rendering the system.
   */
  virtual void finalizeStep(Renderer* renderer) {}

  /** Resets the generator state (e.g. particles, buffers, etc.) */
  virtual void reset() = 0;

  /** Sets the system to be generated. */
  virtual void setSystem(const CameraSystem& system) = 0;

  /**
   * Sets the range of possible particle lifetimes.
   *
   * The first argument is the minimum lifetime, which means the first "min"
   * steps of a particle will be skipped. The second argument is the maximum
   * lifetime, which means that a particle should not be processed after "max"
   * steps.
   * */
  virtual void setLifetimeRange(int32_t min, int32_t max) = 0;

  /** Enables or disables the generator. */
  virtual void setEnabled(bool enabled) { enabled_ = enabled; }

  /** Returns if the generator is enabled. */
  [[nodiscard]] bool isEnabled() const { return enabled_; }

 protected:
  bool enabled_ = false;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_GENERATOR_H
