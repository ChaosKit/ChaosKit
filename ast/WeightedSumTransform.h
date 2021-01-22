#ifndef CHAOSKIT_AST_WEIGHTEDSUMTRANSFORM_H
#define CHAOSKIT_AST_WEIGHTEDSUMTRANSFORM_H

#include "BaseMultiTransform.h"
#include "TransformWithWeight.h"

namespace chaoskit::ast {

class WeightedSumTransform : public BaseMultiTransform {
 public:
  using BaseMultiTransform::BaseMultiTransform;

  WeightedSumTransform() = default;

  WeightedSumTransform(std::vector<Transform> transforms) = delete;

  WeightedSumTransform(const std::vector<Transform>& transforms,
                       std::vector<double> weights)
      : BaseMultiTransform(transforms), weights_(std::move(weights)) {
    assert(transforms.size() == weights.size());
  }

  WeightedSumTransform(std::vector<Transform>&& transforms,
                       std::vector<double>&& weights)
      : BaseMultiTransform(transforms), weights_(weights) {
    assert(transforms.size() == weights.size());
  }

  explicit WeightedSumTransform(std::vector<TransformWithWeight>&& transforms) {
    for (auto it = std::make_move_iterator(transforms.begin()),
              end = std::make_move_iterator(transforms.end());
         it != end; ++it) {
      transforms_.push_back(std::move(it->transform));
      weights_.push_back(it->weight);
    }
  }

  WeightedSumTransform(std::initializer_list<TransformWithWeight> transforms)
      : BaseMultiTransform() {
    for (const auto& [transform, weight] : transforms) {
      transforms_.push_back(transform);
      weights_.push_back(weight);
    }
  }

  [[nodiscard]] const std::vector<double>& weights() const { return weights_; }

  bool operator==(const WeightedSumTransform& other) const {
    return BaseMultiTransform::operator==(other) && other.weights_ == weights_;
  }

 private:
  std::vector<double> weights_;
};

}  // namespace chaoskit::ast

#endif  // CHAOSKIT_AST_WEIGHTEDSUMTRANSFORM_H
