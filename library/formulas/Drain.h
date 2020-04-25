#ifndef CHAOSKIT_LIBRARY_DRAIN_H
#define CHAOSKIT_LIBRARY_DRAIN_H

#include "Formula.h"

namespace chaoskit {
namespace library {

class Drain : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;

    return {plus(input.y(), params[0] * signum(input.x()) *
                                sqrt(abs(params[1] * input.x() - params[2]))),
            params[3] - input.x()};
  }

  uint32_t paramCount() override { return 4; }

  std::vector<float> exampleParams() override {
    return {0.7f, -0.5f, 0.3f, 0.f};
  }
};

}  // namespace library
}  // namespace chaoskit

#endif  // CHAOSKIT_LIBRARY_DRAIN_H
