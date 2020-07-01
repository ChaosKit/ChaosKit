#ifndef CHAOSKIT_CORE_RENDERER_H
#define CHAOSKIT_CORE_RENDERER_H

#include "HistogramBuffer.h"

namespace chaoskit::core {

class Renderer {
 public:
  /** Updates the histogram buffer inside the renderer. */
  virtual void updateHistogramBuffer(const HistogramBuffer& buffer) {}
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_RENDERER_H
