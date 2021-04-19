#include "BlendModel.h"

namespace chaoskit::ui {

BlendModel::BlendModel(ModelFactory *modelFactory, QObject *parent)
    : QObject(parent), BaseModel<Blend>(), modelFactory_(modelFactory) {}

void BlendModel::setProto(Blend *proto) {
  BaseModel::setProto(proto);

  updateNameCache();
  updatePre();
  updatePost();
}

void BlendModel::setEnabled(bool enabled) {
  if (proto_->enabled() == enabled) return;
  proto_->set_enabled(enabled);
  emit enabledChanged();
}

void BlendModel::setWeight(float weight) {
  if (qFuzzyCompare(proto_->weight(), weight)) return;
  proto_->set_weight(weight);
  emit weightChanged();
}

void BlendModel::setName(const QString &name) {
  if (nameCache_ == name) return;

  nameCache_ = name;
  proto_->set_name(name.toStdString());
  emit nameChanged();
}

void BlendModel::addPre() {
  if (pre_ != nullptr) return;

  pre_ = modelFactory_->createTransformModel(this);
  if (proto_->has_pre()) {
    pre_->setProto(proto_->mutable_pre());
  } else {
    pre_->resetProto(proto_->mutable_pre());
  }
  emit preChanged();
}

void BlendModel::removePre() {
  if (pre_ == nullptr) return;

  delete pre_;
  proto_->clear_pre();
  emit preChanged();
}

void BlendModel::addPost() {
  if (post_ != nullptr) return;

  post_ = modelFactory_->createTransformModel(this);
  if (proto_->has_post()) {
    post_->setProto(proto_->mutable_post());
  } else {
    post_->resetProto(proto_->mutable_post());
  }
  emit postChanged();
}

void BlendModel::removePost() {
  if (post_ == nullptr) return;

  delete post_;
  proto_->clear_post();
  emit postChanged();
}

void BlendModel::updatePre() {
  if (proto_->has_pre()) {
    if (pre_ == nullptr) {
      addPre();
    } else {
      pre_->setProto(proto_->mutable_pre());
      emit preChanged();
    }
  } else {
    removePre();
  }
}

void BlendModel::updatePost() {
  if (proto_->has_post()) {
    if (post_ == nullptr) {
      addPost();
    } else {
      post_->setProto(proto_->mutable_post());
      emit postChanged();
    }
  } else {
    removePost();
  }
}

void BlendModel::updateNameCache() {
  QString nameCache = QString::fromStdString(proto_->name());
  if (nameCache_ == nameCache) return;

  nameCache_ = nameCache;
  emit nameChanged();
}

}  // namespace chaoskit::ui
