#include "Formula.h"
#include <library/util.h>
#include <QRandomGenerator>

namespace chaoskit {
namespace ui {

void Formula::setName(const QString &name) {
  if (name_ == name) {
    return;
  }
  name_ = name;
  emit nameChanged();
}

void Formula::setType(const QString &typeName) {
  // throws std::runtime_error if the type is missing
  setType(library::FormulaType::_from_string(typeName.toUtf8()));
}

void Formula::setType(const library::FormulaType &type) {
  if (type_ == type) {
    return;
  }

  auto currentCount = library::createFormula(type_)->paramCount();
  auto newCount = library::createFormula(type)->paramCount();

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
  if (weight_x_ == weight) {
    return;
  }

  weight_x_ = weight;
  emit weightChanged();
}

void Formula::setWeightY(float weight) {
  if (weight_y_ == weight) {
    return;
  }

  weight_y_ = weight;
  emit weightChanged();
}

core::Formula Formula::toCoreFormula() const {
  return {library::createFormula(type_)->source(), params_.toStdVector(),
          weight_x_, weight_y_};
}

}  // namespace ui
}  // namespace chaoskit
