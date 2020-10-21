#ifndef CHAOSKIT_AST_STATICAFFINETRANSFORM_H
#define CHAOSKIT_AST_STATICAFFINETRANSFORM_H

#include <array>
#include <ostream>

namespace chaoskit::ast {

class StaticAffineTransform {
 public:
  StaticAffineTransform() : params_{1.f, 0.f, 0.f, 0.f, 1.f, 0.f} {}
  explicit StaticAffineTransform(const std::array<float, 6> &params)
      : params_(params) {}
  explicit StaticAffineTransform(const float(params)[6])
      : params_{params[0], params[1], params[2],
                params[3], params[4], params[5]} {}
  StaticAffineTransform(float a, float b, float c, float d, float e, float f)
      : params_{a, b, c, d, e, f} {}

  static StaticAffineTransform identity() { return StaticAffineTransform(); }

  [[nodiscard]] const std::array<float, 6> &params() const { return params_; }

  bool operator==(const StaticAffineTransform &other) const {
    return params_ == other.params_;
  }

 private:
  std::array<float, 6> params_;
};

std::ostream &operator<<(std::ostream &stream,
                         const StaticAffineTransform &transform);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_STATICAFFINETRANSFORM_H