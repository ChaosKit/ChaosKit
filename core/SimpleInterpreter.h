#ifndef CHAOSKIT_CORE_SIMPLEINTERPRETER_H
#define CHAOSKIT_CORE_SIMPLEINTERPRETER_H

#include <ast/System.h>
#include "Params.h"
#include "Particle.h"
#include "Rng.h"

namespace chaoskit::core {

class SimpleInterpreter {
 public:
  struct Result {
    Particle next_state;
    Particle output;

    bool operator==(const Result &other) const {
      return next_state == other.next_state && output == other.output;
    }
  };

  explicit SimpleInterpreter(ast::System system, int ttl = Particle::IMMORTAL,
                             Params params = Params{});
  SimpleInterpreter(ast::System system, int ttl, Params params,
                    std::shared_ptr<Rng> rng);

  void setSystem(const ast::System &system);
  void setParams(Params params);
  void setTtl(int ttl);
  Particle randomizeParticle();
  Result operator()(Particle input);

 private:
  ast::System system_;
  int ttl_;
  Params params_;
  float max_limit_;
  std::shared_ptr<Rng> rng_;

  void updateMaxLimit();
  void randomizeParticle(Particle &particle);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SIMPLEINTERPRETER_H
