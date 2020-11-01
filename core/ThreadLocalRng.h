#ifndef CHAOSKIT_CORE_THREADLOCALRNG_H
#define CHAOSKIT_CORE_THREADLOCALRNG_H

#include "Rng.h"

namespace chaoskit::core {

class ThreadLocalRng : public Rng {
 public:
  double randomDouble(double min, double max) override;
  float randomFloat(float min, float max) override;
  int randomInt(int min, int max) override;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_THREADLOCALRNG_H
