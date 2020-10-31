#ifndef CHAOSKIT_CORE_TESTING_STATICRNG_H
#define CHAOSKIT_CORE_TESTING_STATICRNG_H

#include "Rng.h"

class StaticRng : public chaoskit::core::Rng {
 public:
  StaticRng() : f_(0), i_(0) {}
  explicit StaticRng(float f) : f_(f), i_(0) {}
  explicit StaticRng(int i) : f_(0), i_(i) {}
  StaticRng(float f, int i) : f_(f), i_(i) {}

  void setFloat(float f) { f_ = f; }
  void setInt(int i) { i_ = i; }

  float randomFloat(float min, float max) override { return f_; }
  int randomInt(int min, int max) override { return i_; }

 private:
  float f_;
  int i_;
};

#endif  // CHAOSKIT_CORE_TESTING_STATICRNG_H
