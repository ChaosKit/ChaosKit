#include "Formula.h"
#include <library/util.h>
#include <QRandomGenerator>

namespace chaoskit::ui {

void Formula::setType(const QString &typeName) {
  // throws std::runtime_error if the type is missing
  setType(library::FormulaType::_from_string(typeName.toUtf8()));
}

void Formula::setType(const library::FormulaType &type) {
  if (type_ == type) {
    return;
  }

  auto currentCount = library::paramCount(type_);
  auto newCount = library::paramCount(type);

  if (currentCount < newCount) {
    expandParams(newCount);
    emit paramsChanged();
  } else if (currentCount > newCount) {
    params_.resize(newCount);
    // technically the params haven't changed, there's just less of them
  }

  type_ = type;
  emit typeChanged();
}

void Formula::setParams(const QVector<float> &params) {
  if (params.size() < params_.size()) {
    return;
  }

  std::copy(params.begin(), params.begin() + params_.size(), params_.begin());
  emit paramsChanged();
}

void Formula::expandParams(int size) {
  int oldSize = params_.size();
  params_.resize(size);

  auto *rng = QRandomGenerator::global();
  for (auto it = params_.begin() + oldSize; it != params_.end(); it++) {
    *it = static_cast<float>(rng->bounded(4.0) - 2.0);
  }
}

void Formula::setWeightX(float weight) {
  if (qFuzzyCompare(weight_x_, weight)) {
    return;
  }

  weight_x_ = weight;
  emit weightChanged();
}

void Formula::setWeightY(float weight) {
  if (qFuzzyCompare(weight_y_, weight)) {
    return;
  }

  weight_y_ = weight;
  emit weightChanged();
}

}  // namespace chaoskit::ui
