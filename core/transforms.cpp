#include "transforms.h"

namespace chaoskit::core {

Transform rotate(float angle) {
  float sin = sinf(angle);
  float cos = cosf(angle);
  return Transform({cos, -sin, 0, sin, cos, 0});
}

}  // namespace chaoskit::core
