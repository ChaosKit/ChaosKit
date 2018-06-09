#ifndef CHAOSKIT_CORE_RNG_H
#define CHAOSKIT_CORE_RNG_H

namespace chaoskit {
namespace core {

class Rng {
 public:
  virtual float randomFloat(float min, float max) = 0;
  virtual int randomInt(int min, int max) = 0;
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_RNG_H
