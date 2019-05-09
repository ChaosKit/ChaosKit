#ifndef CHAOSKIT_CORE_SIMPLEINTERPRETER_H
#define CHAOSKIT_CORE_SIMPLEINTERPRETER_H

#include <ast/System.h>
#include "Params.h"
#include "Point.h"
#include "Rng.h"

namespace chaoskit::core {

class SimpleInterpreter {
 public:
  struct Result {
    Point next_state;
    Point output;

    bool operator==(const Result &other) const {
      return next_state == other.next_state && output == other.output;
    }
  };

  SimpleInterpreter(ast::System system, Params params = Params{});
  SimpleInterpreter(ast::System system, Params params,
                    std::shared_ptr<Rng> rng);

  void setSystem(const ast::System &system);
  void setParams(Params params);
  Result operator()(Point input);

 private:
  ast::System system_;
  Params params_;
  float max_limit_;
  std::shared_ptr<Rng> rng_;

  void updateMaxLimit();
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SIMPLEINTERPRETER_H
