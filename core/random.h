#ifndef CHAOSKIT_CORE_RANDOM_H
#define CHAOSKIT_CORE_RANDOM_H

#include "randutils.hpp"

namespace chaoskit {
namespace core {

float randomFloat(float min, float max) {
  thread_local randutils::mt19937_rng rng;
  return rng.uniform(min, max);
}

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_RANDOM_H
