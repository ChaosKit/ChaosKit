#include "Lifetime.h"
#include <algorithm>
#include "Rng.h"

namespace chaoskit::core {

Lifetime::Lifetime(int32_t min, int32_t max) { *this = normalize(min, max); }

Lifetime Lifetime::normalize(int32_t min, int32_t max, bool* adjusted) {
  int32_t normalizedMin = min;
  int32_t normalizedMax = max;
  bool wasAdjusted = false;

  if (normalizedMin < 0) {
    normalizedMin = 0;
    wasAdjusted = true;
  }

  if (normalizedMax != INFINITE) {
    if (normalizedMax < 1) {
      normalizedMax = 1;
      wasAdjusted = true;
    }
    if (normalizedMin > normalizedMax) {
      normalizedMin = normalizedMax;
      wasAdjusted = true;
    }
  }

  if (adjusted != nullptr) {
    *adjusted = wasAdjusted;
  }

  Lifetime result;
  result.min_ = normalizedMin;
  result.max_ = normalizedMax;
  return result;
}

int32_t Lifetime::generateInitialLifetime(Rng& rng) const {
  if (isInfinite()) {
    return INFINITE;
  }
  return rng.randomInt((std::min)(min_ + 1, max_), max_);
}

std::ostream& operator<<(std::ostream& stream, const Lifetime& lifetime) {
  if (lifetime.isInfinite()) {
    stream << "[" << lifetime.min() << ", Infinite]";
  } else {
    stream << "[" << lifetime.min() << ", " << lifetime.max() << "]";
  }
  return stream;
}

}  // namespace chaoskit::core
