#ifndef CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H
#define CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H

#include <stdx/optional.h>
#include <vector>

#include "SimpleInterpreter.h"
#include "models/SystemModel.h"

namespace chaoskit::core {

class SimpleHistogramGenerator {
 public:
  SimpleHistogramGenerator(const SystemModel &system, uint32_t width,
                           uint32_t height, std::shared_ptr<Rng> rng);
  SimpleHistogramGenerator(const SystemModel &system, uint32_t width,
                           uint32_t height);

  void setSystem(const SystemModel &system);
  void setSize(uint32_t width, uint32_t height);
  void setIterationCount(uint32_t count);
  void setInfiniteIterationCount();

  const float *data() const { return buffer_.data(); }

  void clear();
  void run();

 private:
  uint32_t width_, height_;
  std::vector<float> buffer_;
  stdx::optional<uint32_t> iteration_count_;
  SimpleInterpreter interpreter_;
  std::shared_ptr<Rng> rng_;

  void add(const Point &point);
  void add(uint32_t x, uint32_t y, float factor = 1);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_SIMPLEHISTOGRAMGENERATOR_H
