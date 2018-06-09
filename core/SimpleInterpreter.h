#ifndef CHAOSKIT_CORE_SIMPLEINTERPRETER_H
#define CHAOSKIT_CORE_SIMPLEINTERPRETER_H

#include "../ast/System.h"
#include "Point.h"
#include "Rng.h"

namespace chaoskit {
namespace core {

class SimpleInterpreter {
 public:
  SimpleInterpreter(ast::System system);
  void initialize(const Point &input);
  void setRng(Rng *rng);
  void setParams(const std::vector<float> &params);
  Point step();

 private:
  std::unique_ptr<Rng> rng_;
  ast::System system_;
  Point state_;
  std::vector<float> params_;
  float max_limit_;
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_SIMPLEINTERPRETER_H
