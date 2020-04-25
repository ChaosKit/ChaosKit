#ifndef CHAOSKIT_LIBRARY_FORMULAS_QUADRATIC_H
#define CHAOSKIT_LIBRARY_FORMULAS_QUADRATIC_H

#include "library/Formula.h"

namespace chaoskit::library {

class Quadratic : public Formula {
 public:
  ast::Formula source() override {
    using namespace ast::helpers;

    // More readable:
    // p0 + p1x + p2xx + p3xy + p4y + p5yy
    // p6 + p7x + p8xx + p9xy + p10y + p11yy

    return {params[0] +
                (params[1] + params[2] * input.x() + params[3] * input.y()) *
                    input.x() +
                (params[4] + params[5] * input.y()) * input.y(),
            params[6] +
                (params[7] + params[8] * input.x() + params[9] * input.y()) *
                    input.x() +
                (params[10] + params[11] * input.y()) * input.y()};
  }

  uint32_t paramCount() override { return 12; }

  std::vector<float> exampleParams() override {
    return {-0.3426902,  0.0111925136,   -0.474016398, -0.365768552,
            -1.11398947, -0.00851197448, 0.252567828,  0.220981747,
            -1.53484643, 0.348529458,    0.737327933,  0.599440217};
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_QUADRATIC_H
