#ifndef CHAOSKIT_CORE_SYSTEMPROCESSOR_H
#define CHAOSKIT_CORE_SYSTEMPROCESSOR_H

#include "Rng.h"
#include "SystemParticle.h"
#include "TransformInterpreter.h"
#include "TransformSystem.h"

namespace chaoskit::core {

class SystemProcessor {
 public:
  struct Bounds {
    float left, right;
    float top, bottom;
  };

  SystemProcessor() = delete;
  SystemProcessor(TransformSystem system, std::shared_ptr<Rng> rng)
      : system_(std::move(system)), interpreter_(rng), rng_(std::move(rng)) {}

  void setSystem(TransformSystem system) { system_ = std::move(system); }
  void setBounds(Bounds bounds) { bounds_ = bounds; }
  void setParticleLifetime(int lifetime) { lifetime_ = lifetime; }
  void setSkip(int skip) { skip_ = skip; }

  SystemParticle createParticle() const;
  SystemParticle process() const;
  SystemParticle process(SystemParticle input) const;

 private:
  TransformSystem system_;
  TransformInterpreter interpreter_;
  std::shared_ptr<Rng> rng_;
  Bounds bounds_ = {-1, 1, -1, 1};
  int lifetime_ = Particle::IMMORTAL;
  int skip_ = 0;

  void reviveParticle(Particle &particle) const;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEMPROCESSOR_H
