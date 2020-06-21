#ifndef CHAOSKIT_CORE_GENERATOR_H
#define CHAOSKIT_CORE_GENERATOR_H

#include "core/structures/System.h"

namespace chaoskit::core {

class Generator {
 public:
  /**
   * Called by the render loop before rendering the system.
   * This is the place to synchronize the current rendering result.
   */
  virtual void beforeRendering() = 0;

  /** Called by the render loop after rendering the system. */
  virtual void afterRendering() {}

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
