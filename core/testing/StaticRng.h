#ifndef CHAOSKIT_CORE_TESTING_STATICRNG_H
#define CHAOSKIT_CORE_TESTING_STATICRNG_H

#include "Rng.h"

class StaticRng : public chaoskit::core::Rng {
 public:
  StaticRng() : f_(0), i_(0) {}
  explicit StaticRng(double d) : d_(d) {}
  explicit StaticRng(float f) : f_(f) {}
  explicit StaticRng(int i) : i_(i) {}

  void setDouble(double d) { d_ = d; }
  void setFloat(float f) { f_ = f; }
  void setInt(int i) { i_ = i; }

  double randomDouble(double min, double max) override { return d_; }
  float randomFloat(float min, float max) override { return f_; }
  int randomInt(int min, int max) override { return i_; }

 private:
  double d_ = 0;
  float f_ = 0;
  int i_ = 0;
};

#endif  // CHAOSKIT_CORE_TESTING_STATICRNG_H
