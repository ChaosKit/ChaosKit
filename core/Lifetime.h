#ifndef CHAOSKIT_CORE_LIFETIME_H
#define CHAOSKIT_CORE_LIFETIME_H

#include <cstdint>
#include "SystemParticle.h"

namespace chaoskit::core {

class Rng;

class Lifetime {
 public:
  static constexpr int32_t INFINITE = SystemParticle::INFINITE_LIFETIME;

  constexpr Lifetime() : min_(0), max_(INFINITE) {}
  Lifetime(int32_t min, int32_t max);

  static Lifetime normalize(int32_t min, int32_t max, bool* adjusted = nullptr);

  [[nodiscard]] constexpr int32_t min() const { return min_; }
  [[nodiscard]] constexpr int32_t max() const { return max_; }
  [[nodiscard]] constexpr bool isInfinite() const { return max_ == INFINITE; }

  [[nodiscard]] int32_t generateInitialLifetime(Rng& rng) const;
  [[nodiscard]] constexpr int32_t revivedLifetime() const { return max_; }

  constexpr bool operator==(const Lifetime& other) const {
    return min_ == other.min_ && max_ == other.max_;
  }
  constexpr bool operator!=(const Lifetime& other) const {
    return !(*this == other);
  }

 private:
  int32_t min_ = 0;
  int32_t max_ = INFINITE;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_LIFETIME_H
