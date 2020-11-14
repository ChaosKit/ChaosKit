#ifndef CHAOSKIT_CORE_TRANSFORMS_H
#define CHAOSKIT_CORE_TRANSFORMS_H

#include "Transform.h"

namespace chaoskit::flame {

constexpr Transform identity() { return Transform({1, 0, 0, 0, 1, 0}); }
constexpr Transform scale(float x, float y) {
  return Transform({x, 0, 0, 0, y, 0});
}
constexpr Transform scale(float xy) { return scale(xy, xy); }
constexpr Transform translate(float x, float y) {
  return Transform({1, 0, x, 0, 1, y});
}

Transform rotate(float angle);

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_CORE_TRANSFORMS_H
