#ifndef CHAOSKIT_CORE_TRANSFORM_H
#define CHAOSKIT_CORE_TRANSFORM_H

#include <cmath>
#include "../ast/Transform.h"

namespace chaoskit {
namespace core {

struct Transform {
  float rotation = 0.f;
  float scale_x = 1.f;
  float scale_y = 1.f;
  float translation_x = 0.f;
  float translation_y = 0.f;

  void setScale(float scale) {
    scale_x = scale;
    scale_y = scale;
  }

  ast::Transform toSource() const {
    auto sin = std::sin(rotation);
    auto cos = std::cos(rotation);

    return ast::Transform{
        scale_x * cos,
        scale_x * -sin,
        scale_x * (translation_x * cos - translation_y * sin),
        scale_y * sin,
        scale_y * cos,
        scale_y * (translation_x * sin + translation_y * cos),
    };
  }
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_TRANSFORM_H
