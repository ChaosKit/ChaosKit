#include "ColoringMethodModel.h"

namespace chaoskit::ui {

static const QStringList TYPES{"None", "Single color", "Distance-based"};

ColoringMethodModel::ColoringMethodModel(QObject *parent)
    : BaseModel<ColoringMethod>(parent) {
  setObjectName(QStringLiteral("coloringMethod"));
}

void ColoringMethodModel::setProto(ColoringMethod *proto) {
  BaseModel::setProto(proto);
  emit protoChanged();
}

const QStringList &ColoringMethodModel::types() const { return TYPES; }

int ColoringMethodModel::typeIndex() const {
  return static_cast<int>(proto_->method_case());
}

void ColoringMethodModel::setTypeIndex(int index) {
  if (index == typeIndex()) return;

  switch (index) {
    case 0:
      proto_->clear_method();
      break;
    case 1:
      proto_->set_single_color(.5f);
      break;
    case 2:
      proto_->set_distance(.1f);
      break;
    default:
      // Don't do any change.
      return;
  }

  emit protoChanged();
  emit typeIndexChanged();
  emit parameterChanged();
}

float ColoringMethodModel::parameter() const {
  switch (proto_->method_case()) {
    case ColoringMethod::METHOD_NOT_SET:
      return 0.f;
    case ColoringMethod::kSingleColor:
      return proto_->single_color();
    case ColoringMethod::kDistance:
      return proto_->distance();
  }
}

void ColoringMethodModel::setParameter(float param) {
  switch (proto_->method_case()) {
    case ColoringMethod::METHOD_NOT_SET:
      // No parameter supported.
      return;
    case ColoringMethod::kSingleColor:
      proto_->set_single_color(param);
      break;
    case ColoringMethod::kDistance:
      proto_->set_distance(param);
      break;
  }

  emit protoChanged();
  emit parameterChanged();
}

}  // namespace chaoskit::ui
