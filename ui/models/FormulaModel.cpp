#include "FormulaModel.h"
#include <QRandomGenerator64>
#include <algorithm>
#include <magic_enum.hpp>
#include "library/util.h"

namespace chaoskit::ui {

FormulaModel::FormulaModel(QObject *parent) : BaseModel(parent) {
  setObjectName("formula");
}

void FormulaModel::setProto(Formula *proto) {
  BaseModel::setProto(proto);

  updateTypeCache();
  ensureParamCount();
  updateParamsCache();

  emit protoChanged();
}

void FormulaModel::setType(const QString &type) {
  if (typeCache_ == type) return;

  // Validate the input.
  auto typeStr = type.toStdString();
  auto typeEnum = magic_enum::enum_cast<library::FormulaType>(typeStr);
  if (!typeEnum) {
    emit invalidTypePicked();
    return;
  }

  // Set the type.
  type_ = *typeEnum;
  typeCache_ = type;
  proto_->set_type(std::move(typeStr));

  // Resize the params.
  bool paramsModified = ensureParamCount();
  if (paramsModified) {
    updateParamsCache();
  }

  emit protoChanged();
  emit typeChanged();
}

void FormulaModel::setParam(int i, float value) {
  if (qFuzzyCompare(paramsCache_[i].toFloat(), value)) return;

  paramsCache_[i] = value;
  proto_->set_params(i, value);
  emit protoChanged();
  emit paramsChanged();
}

void FormulaModel::randomizeParams() {
  auto rng = QRandomGenerator64::securelySeeded();
  std::uniform_real_distribution<float> distribution(-1, 1);

  int i = 0;
  for (auto &param : paramsCache_) {
    float value = distribution(rng);
    param.setValue(value);
    proto_->set_params(i, value);
    i++;
  }

  emit protoChanged();
  emit paramsChanged();
}

void FormulaModel::setWeight(float weight) {
  if (qFuzzyCompare(this->weight(), weight)) return;

  auto *w = proto_->mutable_weight();
  w->set_x(weight);
  w->set_y(weight);
  emit protoChanged();
  emit weightChanged();
}

void FormulaModel::updateTypeCache() {
  auto type = magic_enum::enum_cast<library::FormulaType>(proto_->type())
                  .value_or(library::FormulaType::Invalid);
  if (type == type_) return;

  type_ = type;
  typeCache_ = QString::fromStdString(proto_->type());
  emit typeChanged();
}

bool FormulaModel::ensureParamCount() {
  int desiredParamCount = static_cast<int>(library::paramCount(type_));
  int actualParamsCount = proto_->params_size();
  if (actualParamsCount == desiredParamCount) return false;

  auto rng = QRandomGenerator64::securelySeeded();
  std::uniform_real_distribution<float> distribution(-1, 1);

  if (actualParamsCount < desiredParamCount) {
    int difference = desiredParamCount - actualParamsCount;
    while (difference--) {
      proto_->add_params(distribution(rng));
    }
  } else {
    int difference = actualParamsCount - desiredParamCount;
    while (difference--) {
      proto_->mutable_params()->RemoveLast();
    }
  }

  // Remember to emit protoChanged after this returns true!
  return true;
}

void FormulaModel::updateParamsCache() {
  if (proto_->params_size() == paramsCache_.size() &&
      std::equal(paramsCache_.begin(), paramsCache_.end(),
                 proto_->params().begin(), [](const QVariant &a, float b) {
                   return qFuzzyCompare(a.toFloat(), b);
                 })) {
    return;
  }

  paramsCache_.clear();
  for (float param : proto_->params()) {
    paramsCache_.append(param);
  }
  emit paramsChanged();
}

}  // namespace chaoskit::ui
