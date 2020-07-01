#ifndef CHAOSKIT_CORE_GENERATOR_H
#define CHAOSKIT_CORE_GENERATOR_H

#include "Renderer.h"
#include "core/structures/System.h"

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
  virtual void setSystem(const System& system) = 0;

  /** Enables or disables the generator. */
  virtual void setEnabled(bool enabled) { enabled_ = enabled; }

  /** Returns if the generator is enabled. */
  [[nodiscard]] bool isEnabled() const { return enabled_; }

 protected:
  bool enabled_ = false;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_GENERATOR_H
