#ifndef CHAOSKIT_LIBRARY_LINEAR_H
#define CHAOSKIT_LIBRARY_LINEAR_H

#include "Formula.h"

namespace chaoskit {
namespace library {

class Linear : public Formula {
 public:
  ast::Formula source() override {
    return {input.x(), input.y()};
  }

  uint32_t paramCount() override { return 0; }
};

}  // namespace library
}  // namespace chaoskit

#endif //CHAOSKIT_LIBRARY_LINEAR_H
