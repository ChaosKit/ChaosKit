#ifndef CHAOSKIT_AST_LIMITEDBLEND_H
#define CHAOSKIT_AST_LIMITEDBLEND_H

#include <ostream>
#include "Blend.h"

namespace chaoskit::ast {

class LimitedBlend {
 public:
  LimitedBlend(Blend blend, float limit)
      : blend_(std::move(blend)), limit_(limit) {}

  [[nodiscard]] const Blend &blend() const { return blend_; }
  [[nodiscard]] float limit() const { return limit_; }

  bool operator==(const LimitedBlend &other) const {
    return blend_ == other.blend_ && limit_ == other.limit_;
  }

 private:
  Blend blend_;
  float limit_;
};

std::ostream &operator<<(std::ostream &stream, const LimitedBlend &blend);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_LIMITEDBLEND_H
