#include "TransformModel.h"

#include <QTransform>
#include <cmath>

namespace chaoskit::ui {

TransformModel::TransformModel(QObject *parent) : BaseModel<Transform>(parent) {
  setObjectName("transform");

  // Emit changed signals on all properties when transformChanged emits.
  connect(this, &TransformModel::transformChanged, this,
          &TransformModel::scaleXChanged);
  connect(this, &TransformModel::transformChanged, this,
          &TransformModel::scaleYChanged);
  connect(this, &TransformModel::transformChanged, this,
          &TransformModel::rotationChanged);
  connect(this, &TransformModel::transformChanged, this,
          &TransformModel::translationXChanged);
  connect(this, &TransformModel::transformChanged, this,
          &TransformModel::translationYChanged);
}

void TransformModel::setProto(Transform *proto) {
  BaseModel::setProto(proto);

  // Convert the matrix that's stored in the proto to human-readable parameters.
  translationX_ = proto->m31();
  translationY_ = proto->m32();
  scaleX_ =
      std::sqrt(proto->m11() * proto->m11() + proto->m12() * proto->m12());
  scaleY_ =
      std::sqrt(proto->m21() * proto->m21() + proto->m22() * proto->m22());
  rotation_ =
      std::atan2(proto->m21() / scaleY_, proto->m11() / scaleX_) * 180 / M_PI;

  emit transformChanged();
  emit protoChanged();
}

void TransformModel::setScaleX(float sx) {
  if (qFuzzyCompare(scaleX_, sx)) return;
  scaleX_ = sx;
  updateMatrix();
  emit scaleXChanged();
}

void TransformModel::setScaleY(float sy) {
  if (qFuzzyCompare(scaleY_, sy)) return;
  scaleY_ = sy;
  updateMatrix();
  emit scaleYChanged();
}

void TransformModel::setRotation(float angle) {
  if (qFuzzyCompare(rotation_, angle)) return;
  rotation_ = angle;
  updateMatrix();
  emit rotationChanged();
}

void TransformModel::setTranslationX(float dx) {
  if (qFuzzyCompare(translationX_, dx)) return;
  translationX_ = dx;
  updateMatrix();
  emit translationXChanged();
}

void TransformModel::setTranslationY(float dy) {
  if (qFuzzyCompare(translationY_, dy)) return;
  translationY_ = dy;
  updateMatrix();
  emit translationYChanged();
}

void TransformModel::reset() {
  proto_->set_m11(1);
  proto_->set_m21(0);
  proto_->set_m31(0);
  proto_->set_m21(0);
  proto_->set_m22(1);
  proto_->set_m32(0);

  // Trigger an update of all properties.
  setProto(proto_);
}

void TransformModel::resetProto(Transform *proto) {
  // Set all fields to defaults.
  proto->set_m11(1);
  proto->set_m21(0);
  proto->set_m31(0);
  proto->set_m21(0);
  proto->set_m22(1);
  proto->set_m32(0);

  // Set the proto.
  setProto(proto);
}

void TransformModel::updateMatrix() {
  auto transform = QTransform::fromTranslate(translationX_, translationY_)
                       .rotate(rotation_)
                       .scale(scaleX_, scaleY_);
  proto_->set_m11(transform.m11());
  proto_->set_m21(transform.m21());
  proto_->set_m31(transform.m31());
  proto_->set_m21(transform.m21());
  proto_->set_m22(transform.m22());
  proto_->set_m32(transform.m32());
}

}  // namespace chaoskit::ui
