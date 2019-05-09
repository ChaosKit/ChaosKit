#ifndef CHAOSKIT_CORE_TRANSFORM_H
#define CHAOSKIT_CORE_TRANSFORM_H

#include <ast/Transform.h>
#include <cmath>

namespace chaoskit::core {

struct Transform {
  float angle = 0.f;
  float scale_x = 1.f;
  float scale_y = 1.f;
  float translation_x = 0.f;
  float translation_y = 0.f;

  static Transform identity() { return Transform{}; }
  static Transform fromRotation(float radians) {
    return Transform{}.rotate(radians);
  }
  static Transform fromScale(float scale) {
    return Transform{}.setScale(scale);
  }
  static Transform fromScale(float x, float y) {
    return Transform{}.scale(x, y);
  }
  static Transform fromTranslation(float x, float y) {
    return Transform{}.translate(x, y);
  }

  Transform &setScale(float scale) {
    scale_x = scale;
    scale_y = scale;
    return *this;
  }
  Transform &rotate(float radians) {
    angle += radians;
    return *this;
  }
  Transform &scale(float scale) {
    scale_x *= scale;
    scale_y *= scale;
    return *this;
  }
  Transform &scale(float x, float y) {
    scale_x *= x;
    scale_y *= y;
    return *this;
  }
  Transform &translate(float x, float y) {
    translation_x += x;
    translation_y += y;
    return *this;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TRANSFORM_H
