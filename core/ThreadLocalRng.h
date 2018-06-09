#ifndef CHAOSKIT_CORE_THREADLOCALRNG_H
#define CHAOSKIT_CORE_THREADLOCALRNG_H

#include "Rng.h"

namespace chaoskit {
namespace core {

class ThreadLocalRng : public Rng {
 public:
  float randomFloat(float min, float max) override;
  int randomInt(int min, int max) override;
};

}
}

#endif //CHAOSKIT_CORE_THREADLOCALRNG_H
