#ifndef CHAOSKIT_CORE_COLORMAP_H
#define CHAOSKIT_CORE_COLORMAP_H

#include "Color.h"

namespace chaoskit::core {

class ColorMap {
 public:
  virtual ~ColorMap() = default;
  [[nodiscard]] virtual Color map(float color) const = 0;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_COLORMAP_H
