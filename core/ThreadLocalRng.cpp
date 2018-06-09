#include "ThreadLocalRng.h"

#include "randutils.hpp"

namespace chaoskit {
namespace core {

namespace {
thread_local randutils::mt19937_rng rng;
}

float ThreadLocalRng::randomFloat(float min, float max) {
  return rng.uniform(min, max);
}

int ThreadLocalRng::randomInt(int min, int max) {
  return rng.uniform(min, max);
}

}
}
