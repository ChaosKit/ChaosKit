#include "ThreadLocalRng.h"

#include <pcg_random.hpp>
#include <random>

namespace chaoskit::core {

namespace {
thread_local pcg32 rng{pcg_extras::seed_seq_from<std::random_device>()};
}  // namespace

double ThreadLocalRng::randomDouble(double min, double max) {
  std::uniform_real_distribution<double> distribution(min, max);
  return distribution(rng);
}
float ThreadLocalRng::randomFloat(float min, float max) {
  std::uniform_real_distribution<float> distribution(min, max);
  return distribution(rng);
}
int ThreadLocalRng::randomInt(int min, int max) {
  std::uniform_int_distribution<int> distribution(min, max);
  return distribution(rng);
}

}  // namespace chaoskit::core
