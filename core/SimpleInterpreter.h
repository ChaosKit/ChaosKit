#ifndef CHAOSKIT_CORE_SIMPLEINTERPRETER_H
#define CHAOSKIT_CORE_SIMPLEINTERPRETER_H

#include "../ast/System.h"
#include "Point.h"
#include "Rng.h"

namespace chaoskit {
namespace core {

class SimpleInterpreter {
 public:
  struct Result {
    Point next_state;
    Point output;

    bool operator==(const Result &other) const {
      return next_state == other.next_state && output == other.output;
    }
  };

  SimpleInterpreter(ast::System system, std::vector<float> params = {});
  SimpleInterpreter(ast::System system, std::vector<float> params, std::shared_ptr<Rng> rng);

  void setSystem(const ast::System &system);
  void setParams(const std::vector<float> &params);
  Result step(Point input);

 private:
  ast::System system_;
  std::vector<float> params_;
  float max_limit_;
  std::shared_ptr<Rng> rng_;

  void updateMaxLimit();
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_SIMPLEINTERPRETER_H
