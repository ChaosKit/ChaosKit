#ifndef CHAOSKIT_CORE_SYSTEMPROCESSOR_H
#define CHAOSKIT_CORE_SYSTEMPROCESSOR_H

#include "Lifetime.h"
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

  SystemProcessor(ast::Transform transform, std::shared_ptr<Rng> rng)
      : system_{std::move(transform), {}},
        interpreter_(rng),
        rng_(std::move(rng)) {}

  SystemProcessor(ast::Transform transform, TransformParams params,
                  std::shared_ptr<Rng> rng)
      : system_{std::move(transform), std::move(params)},
        interpreter_(rng),
        rng_(std::move(rng)) {}

  SystemProcessor(TransformSystem system, std::shared_ptr<Rng> rng)
      : system_(std::move(system)), interpreter_(rng), rng_(std::move(rng)) {}

  void setSystem(TransformSystem system) { system_ = std::move(system); }
  void setBounds(Bounds bounds) { bounds_ = bounds; }
  void setLifetime(Lifetime lifetime) { lifetime_ = lifetime; }
  [[nodiscard]] const Lifetime &lifetime() const { return lifetime_; }
  void setMinLifetime(int32_t min) {
    lifetime_ = Lifetime(min, lifetime_.max());
  }
  void setMaxLifetime(int32_t max) {
    lifetime_ = Lifetime(lifetime_.min(), max);
  }
  void setLifetimeRange(int32_t min, int32_t max) {
    lifetime_ = Lifetime(min, max);
  }

  [[nodiscard]] SystemParticle createParticle() const;
  /**
   * Advances the particle through the system.
   * If particle.warmup > 0, executes that many unplotted warm-up iterations
   * before performing and returning a single plotted iteration.
   */
  [[nodiscard]] SystemParticle process() const;
  [[nodiscard]] SystemParticle process(SystemParticle input) const;

 protected:
  TransformInterpreter interpreter_;

 private:
  TransformSystem system_;
  std::shared_ptr<Rng> rng_;
  Bounds bounds_ = {-1, 1, -1, 1};
  Lifetime lifetime_ = Lifetime();

  void reviveParticle(Particle &particle) const;
  void stepParticle(SystemParticle &particle) const;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SYSTEMPROCESSOR_H
