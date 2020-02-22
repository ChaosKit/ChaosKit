#include "RainbowColorMap.h"

#include <cmath>

namespace chaoskit::core {

namespace {

constexpr double PI_3 = M_PI / 3.0;

Color fromHsv(double h, double s, double v) {
  double c = v * s;
  double x = c * (1 - std::fabs(std::fmod(h / PI_3, 2) - 1));
  double m = v - c;

  double r, g, b;
  if (h < PI_3) {
    r = c;
    g = x;
    b = 0;
  } else if (h < PI_3 * 2) {
    r = x;
    g = c;
    b = 0;
  } else if (h < M_PI) {
    r = 0;
    g = c;
    b = x;
  } else if (h < PI_3 * 4) {
    r = 0;
    g = x;
    b = c;
  } else if (h < PI_3 * 5) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }

  return {static_cast<float>(r + m), static_cast<float>(g + m),
          static_cast<float>(b + m)};
}

}  // namespace

Color RainbowColorMap::map(float color) const {
  return fromHsv(color * M_PI * 2, 0.7, 0.9);
}

}  // namespace chaoskit::core
