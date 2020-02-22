#ifndef CHAOSKIT_CORE_COLOR_H
#define CHAOSKIT_CORE_COLOR_H

namespace chaoskit::core {

struct Color {
  Color() : r(0.f), g(0.f), b(0.f), a(1.f) {}
  explicit Color(float grey) : r(grey), g(grey), b(grey), a(1.f) {}
  Color(float r, float g, float b, float a = 1.f) : r(r), g(g), b(b), a(a) {}

  float r, g, b, a;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_COLOR_H
