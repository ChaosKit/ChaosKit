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
    return {0.01031494140625,     1.9098844528198242,   0.032599925994873047,
            1.628016471862793,    1.9790589809417725,   -0.84788751602172852,
            -0.37978851795196533, -0.77007508277893066, -0.29285454750061035,
            -0.38380205631256104, 1.4894475936889648,   1.1234347820281982};
  }
};

}  // namespace chaoskit::library

#endif  // CHAOSKIT_LIBRARY_FORMULAS_QUADRATIC_H
