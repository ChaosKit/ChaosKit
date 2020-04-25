#ifndef CHAOSKIT_LIBRARY_DEJONG_H
#define CHAOSKIT_LIBRARY_DEJONG_H

#include "library/Formula.h"

namespace chaoskit {
namespace library {

class DeJong : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    return {sin(params[0] * input.y()) - cos(params[1] * input.x()),
            sin(params[2] * input.x()) - cos(params[3] * input.y())};
  }

  uint32_t paramCount() override { return 4; }

  std::vector<float> exampleParams() override {
    return {-1.9292301883127383f, -1.7559409159631594f, -1.8413772506711874f,
            -1.972643807513176f};
  }
};

}  // namespace library
}  // namespace chaoskit

#endif  // CHAOSKIT_LIBRARY_DEJONG_H
