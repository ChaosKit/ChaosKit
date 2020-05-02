#ifndef CHAOSKIT_CORE_TRANSFORM_H
#define CHAOSKIT_CORE_TRANSFORM_H

#include <array>
#include <cmath>

namespace chaoskit::core {

struct Transform {
  std::array<float, 6> values;

  constexpr Transform() : values{1, 0, 0, 0, 1, 0} {}
  constexpr explicit Transform(std::array<float, 6> &&values)
      : values(std::forward<std::array<float, 6>>(values)) {}
  Transform(const Transform &other) = default;
  Transform &operator=(const Transform &other) = default;

  Transform operator*(const Transform &other) const {
    Transform newTransform(*this);
    newTransform *= other;
    return newTransform;
  }

  Transform &operator*=(const Transform &other) {
    values = {
        values[0] * other.values[0] + values[1] * other.values[3],
        values[0] * other.values[1] + values[1] * other.values[4],
        values[0] * other.values[2] + values[1] * other.values[5] + values[2],
        values[3] * other.values[0] + values[4] * other.values[3],
        values[3] * other.values[1] + values[4] * other.values[4],
        values[3] * other.values[2] + values[4] * other.values[5] + values[5],
    };
    return *this;
  }
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TRANSFORM_H
