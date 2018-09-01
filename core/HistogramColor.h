#ifndef CHAOSKIT_CORE_HISTOGRAMCOLOR_H
#define CHAOSKIT_CORE_HISTOGRAMCOLOR_H

namespace chaoskit {
namespace core {

struct HistogramColor {
  float red, green, blue;
  float intensity;

  void operator+=(const HistogramColor &color) {
    red += color.red;
    green += color.green;
    blue += color.blue;
    intensity += color.intensity;
  }

  static HistogramColor zero() { return HistogramColor{0.f, 0.f, 0.f, 0.f}; }
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_HISTOGRAMCOLOR_H
