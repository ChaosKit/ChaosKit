#ifndef CHAOSKIT_CORE_SIMPLEINTERPRETER_H
#define CHAOSKIT_CORE_SIMPLEINTERPRETER_H

#include <ast/System.h>
#include "Particle.h"
#include "Rng.h"
#include "flame/Params.h"
#include "flame/System.h"
#include "flame/Transform.h"

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

  SimpleInterpreter();
  explicit SimpleInterpreter(
      ast::System system, int ttl = Particle::IMMORTAL, int skip = 0,
      flame::Params params = flame::Params{},
      flame::Transform initialTransform = flame::Transform());
  SimpleInterpreter(ast::System system, int ttl, int skip,
                    chaoskit::flame::Params params,
                    flame::Transform initialTransform,
                    std::shared_ptr<Rng> rng);
  explicit SimpleInterpreter(const flame::System &system);
  SimpleInterpreter(const flame::System &system, std::shared_ptr<Rng> rng);

  void setSystem(const ast::System &system);
  void setSystem(const flame::System &system);
  void setParams(flame::Params params);
  void setTtl(int ttl);
  void setSkip(int skip);
  void setInitialTransform(flame::Transform transform);
  Particle randomizeParticle() const;
  Particle processBlends(Particle input) const;
  Particle processFinalBlend(Particle input) const;

  Result operator()(Particle input) const;

 private:
  ast::System system_;
  int ttl_;
  int skip_;
  flame::Params params_;
  flame::Transform initialTransform_;
  float max_limit_;
  std::shared_ptr<Rng> rng_;

  void updateMaxLimit();
  void randomizeParticle(Particle &particle) const;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SIMPLEINTERPRETER_H
