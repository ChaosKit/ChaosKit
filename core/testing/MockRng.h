#ifndef CHAOSKIT_CORE_TESTING_MOCKRNG_H
#define CHAOSKIT_CORE_TESTING_MOCKRNG_H

#include <gmock/gmock.h>
#include "Rng.h"

namespace chaoskit::core {

class MockRng : public Rng {
 public:
  MOCK_METHOD(double, randomDouble, (double min, double max), (override));
  MOCK_METHOD(float, randomFloat, (float min, float max), (override));
  MOCK_METHOD(int, randomInt, (int min, int max), (override));
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TESTING_MOCKRNG_H
