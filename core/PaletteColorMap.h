#ifndef CHAOSKIT_CORE_PALETTECOLORMAP_H
#define CHAOSKIT_CORE_PALETTECOLORMAP_H

#include <utility>
#include <vector>
#include "Color.h"
#include "ColorMap.h"

namespace chaoskit::core {

class PaletteColorMap : public ColorMap {
 public:
  explicit PaletteColorMap(std::vector<Color> palette)
      : palette_(std::move(palette)) {}

  [[nodiscard]] Color map(float color) const override;

 private:
  std::vector<Color> palette_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_PALETTECOLORMAP_H
