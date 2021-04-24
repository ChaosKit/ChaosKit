#include "FormulaModel.h"
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

  // Resize the params.
  int newParamCount = static_cast<int>(library::paramCount(*typeEnum));
  int countDiff = newParamCount - paramsCache_.size();
  bool resized = countDiff != 0;
  proto_->mutable_params()->Resize(newParamCount, 0.f);
  if (countDiff > 0) {
    while (countDiff--) paramsCache_.append(0.f);
  } else if (countDiff < 0) {
    while (countDiff++) paramsCache_.removeLast();
  }
  if (resized) {
    emit paramsChanged();
  }

  // Set the type.
  type_ = *typeEnum;
  typeCache_ = type;
  proto_->set_type(std::move(typeStr));
  emit protoChanged();
  emit typeChanged();
}

void FormulaModel::setParam(int i, float value) {
  if (qFuzzyCompare(paramsCache_[i].toFloat(), value)) return;

  paramsCache_[i] = value;
  proto_->mutable_params()->Set(i, value);
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

void FormulaModel::updateParamsCache() {
  if (std::equal(paramsCache_.begin(), paramsCache_.end(),
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
