#ifndef CHAOSKIT_AST_SYSTEM_H
#define CHAOSKIT_AST_SYSTEM_H

#include <ostream>
#include <vector>
#include "LimitedBlend.h"

namespace chaoskit::ast {

class System {
 public:
  explicit System(std::vector<LimitedBlend> blends = {})
      : blends_(std::move(blends)), final_blend_() {}
  System(std::vector<LimitedBlend> blends, Blend final_blend)
      : blends_(std::move(blends)), final_blend_(std::move(final_blend)) {}

  [[nodiscard]] const std::vector<LimitedBlend> &blends() const {
    return blends_;
  }
  [[nodiscard]] const Blend &final_blend() const { return final_blend_; }

  bool operator==(const System &other) const {
    return blends_ == other.blends_ && final_blend_ == other.final_blend_;
  }

 private:
  std::vector<LimitedBlend> blends_;
  Blend final_blend_;
};

std::ostream &operator<<(std::ostream &stream, const System &system);

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_SYSTEM_H
