#ifndef CHAOSKIT_CLI_SEEDEDRNG_H
#define CHAOSKIT_CLI_SEEDEDRNG_H

#include <pcg_random.hpp>
#include <random>
#include "core/Rng.h"

namespace chaoskit::cli {

class SeededRng : public core::Rng {
 public:
  explicit SeededRng(uint64_t seed) : rng_(seed) {}

  double randomDouble(double min, double max) override {
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(rng_);
  }

  float randomFloat(float min, float max) override {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(rng_);
  }

  int randomInt(int min, int max) override {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(rng_);
  }

 private:
  pcg32 rng_;
};

}  // namespace chaoskit::cli

#endif  // CHAOSKIT_CLI_SEEDEDRNG_H
