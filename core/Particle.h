#ifndef CHAOSKIT_CORE_PARTICLE_H
#define CHAOSKIT_CORE_PARTICLE_H

#include "Point.h"

namespace chaoskit {
namespace core {

struct Particle {
  static const int32_t IMMORTAL = -1;

  Point point;
  float color = .5f;
  int32_t ttl = IMMORTAL;
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_PARTICLE_H
