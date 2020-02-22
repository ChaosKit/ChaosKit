#ifndef CHAOSKIT_CORE_RAINBOWCOLORMAP_H
#define CHAOSKIT_CORE_RAINBOWCOLORMAP_H

#include "ColorMap.h"

namespace chaoskit::core {

class RainbowColorMap : public ColorMap {
 public:
  [[nodiscard]] Color map(float color) const override;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_RAINBOWCOLORMAP_H
