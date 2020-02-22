#ifndef CHAOSKIT_CORE_BLACKWHITECOLORMAP_H
#define CHAOSKIT_CORE_BLACKWHITECOLORMAP_H

#include "ColorMap.h"

namespace chaoskit::core {

class BlackWhiteColorMap : public ColorMap {
 public:
  [[nodiscard]] Color map(float color) const override { return Color(color); }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_BLACKWHITECOLORMAP_H
