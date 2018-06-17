#ifndef CHAOSKIT_CORE_PARAMS_H
#define CHAOSKIT_CORE_PARAMS_H

#include <unordered_map>
#include <vector>
#include "SystemIndex.h"

namespace chaoskit {
namespace core {

class Params {
 public:
  Params() : values_() {}

  std::vector<float> &operator[](const SystemIndex &index) {
    return values_[index];
  }
  std::vector<float> &operator[](SystemIndex &&index) { return values_[index]; }
  std::vector<float> &at(const SystemIndex &index) { return values_.at(index); }
  const std::vector<float> &at(const SystemIndex &index) const {
    return values_.at(index);
  }

 private:
  std::unordered_map<SystemIndex, std::vector<float>> values_;
};

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_PARAMS_H
