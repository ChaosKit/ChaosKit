#include "ThreadLocalRng.h"

#include "randutils.hpp"

namespace chaoskit::core {

namespace {
thread_local randutils::mt19937_rng rng;
}

double ThreadLocalRng::randomDouble(double min, double max) {
  return rng.uniform(min, max);
}
float ThreadLocalRng::randomFloat(float min, float max) {
  return rng.uniform(min, max);
}
int ThreadLocalRng::randomInt(int min, int max) {
  return rng.uniform(min, max);
}

}  // namespace chaoskit::core
