#ifndef CHAOSKIT_AST_RANDOMCHOICETRANSFORM_H
#define CHAOSKIT_AST_RANDOMCHOICETRANSFORM_H

#include <utility>

#include "BaseMultiTransform.h"
#include "TransformWithWeight.h"

namespace chaoskit::ast {

class RandomChoiceTransform : public BaseMultiTransform {
 public:
  using BaseMultiTransform::BaseMultiTransform;

  RandomChoiceTransform() = default;

  RandomChoiceTransform(std::vector<Transform> transforms) = delete;

  RandomChoiceTransform(const std::vector<Transform>& transforms,
                        std::vector<double> weights)
      : BaseMultiTransform(transforms), weights_(std::move(weights)) {
    assert(transforms_.size() == weights_.size());
  }

  RandomChoiceTransform(std::vector<Transform>&& transforms,
                        std::vector<double>&& weights)
      : BaseMultiTransform(transforms), weights_(weights) {
    assert(transforms_.size() == weights_.size());
  }

  explicit RandomChoiceTransform(
      std::vector<TransformWithWeight>&& transforms) {
    for (auto it = std::make_move_iterator(transforms.begin()),
              end = std::make_move_iterator(transforms.end());
         it != end; ++it) {
      transforms_.push_back(std::move(it->transform));
      weights_.push_back(it->weight);
    }
  }

  RandomChoiceTransform(std::initializer_list<TransformWithWeight> transforms)
      : BaseMultiTransform() {
    for (const auto& [transform, weight] : transforms) {
      transforms_.push_back(transform);
      weights_.push_back(weight);
    }
  }

  [[nodiscard]] const std::vector<double>& weights() const { return weights_; }

  bool operator==(const RandomChoiceTransform& other) const {
    return BaseMultiTransform::operator==(other) && other.weights_ == weights_;
  }

 private:
  std::vector<double> weights_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_RANDOMCHOICETRANSFORM_H
