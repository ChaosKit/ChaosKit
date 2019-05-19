#ifndef CHAOSKIT_CORE_MODELS_MODEL_H
#define CHAOSKIT_CORE_MODELS_MODEL_H

#include <memory>

#define CHAOSKIT_MODEL_CONSTRUCTORS(M)                                 \
  explicit(M)(const std::shared_ptr<Structure> &data) : Model(data) {} \
  explicit(M)(const std::weak_ptr<Structure> &data) : Model(data) {}

namespace chaoskit::core {

template <typename T>
class Model {
 public:
  using Structure = T;

  Model() = default;
  explicit Model(const std::shared_ptr<Structure> &data) : data_(data) {}
  explicit Model(const std::weak_ptr<Structure> &data) : data_(data) {}

  std::weak_ptr<Structure> data() const { return data_; }

 protected:
  std::weak_ptr<Structure> data_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MODELS_MODEL_H
