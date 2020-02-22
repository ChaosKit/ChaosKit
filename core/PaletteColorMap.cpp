#include "PaletteColorMap.h"
#include <cmath>

namespace chaoskit::core {

Color PaletteColorMap::map(float c) const {
  double fractionalIndex = (double)c * static_cast<double>(palette_.size() - 1);
  double index;
  double remainder = modf(fractionalIndex, &index);

  Color color = palette_[static_cast<size_t>(index)];
  if (remainder > 0.0) {
    const Color& next = palette_[static_cast<size_t>(index) + 1];
    color = Color{
        static_cast<float>(color.r * (1 - remainder) + next.r * remainder),
        static_cast<float>(color.g * (1 - remainder) + next.g * remainder),
        static_cast<float>(color.b * (1 - remainder) + next.b * remainder)};
  }
  return color;
}

}  // namespace chaoskit::core
