#ifndef CHAOSKIT_LIBRARY_DEJONG_H
#define CHAOSKIT_LIBRARY_DEJONG_H

#include "Formula.h"

namespace chaoskit {
namespace library {

class DeJong : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;
    return {sin(params[0] * input.y()) - cos(params[1] * input.x()),
            sin(params[2] * input.x()) - cos(params[3] * input.y())};
  }

  uint32_t paramCount() override {
    return 4;
  }
};

}  // namespace library
}  // namespace chaoskit

#endif  // CHAOSKIT_LIBRARY_DEJONG_H
