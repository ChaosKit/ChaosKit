#include "BlendModel.h"

namespace chaoskit::ui {

BlendModel::BlendModel(ModelFactory *modelFactory, QObject *parent)
    : BaseModel<Blend>(parent), modelFactory_(modelFactory) {
  setObjectName(QStringLiteral("blend"));
  coloringMethod_ = modelFactory_->createColoringMethodModel(this);

  // Propagate changes up the hierarchy.
  connect(coloringMethod_, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);
  connect(coloringMethod_, &AbstractBaseModel::protoChanged, this,
          &BlendModel::visuallyChanged);

  formulas_ = new QQmlObjectListModel<FormulaModel>(this);
  formulas_->setObjectName("formulas");
}

void BlendModel::setProto(Blend *proto) {
  BaseModel::setProto(proto);

  updateNameCache();
  updatePre();
  updatePost();

  coloringMethod_->setProto(proto->mutable_coloring_method());

  // Clear old formulas
  for (auto *formula : *formulas_) {
    formula->disconnect(this);
  }
  formulas_->clear();

  // Create new formula models
  QList<FormulaModel *> newModels;
  for (auto &formula : *proto_->mutable_formulas()) {
    auto *model = modelFactory_->createFormulaModel(formulas_);
    model->setProto(&formula);
    connect(model, &AbstractBaseModel::protoChanged, this,
            &AbstractBaseModel::protoChanged);
    // All formula proto changes are visual.
    connect(model, &AbstractBaseModel::protoChanged, this,
            &BlendModel::visuallyChanged);
    newModels.append(model);
  }
  formulas_->append(newModels);

  emit protoChanged();
  emit visuallyChanged();
}

void BlendModel::setEnabled(bool enabled) {
  if (proto_->enabled() == enabled) return;
  proto_->set_enabled(enabled);
  emit protoChanged();
  emit visuallyChanged();
  emit enabledChanged();
}

void BlendModel::setWeight(float weight) {
  if (qFuzzyCompare(proto_->weight(), weight)) return;
  proto_->set_weight(weight);
  emit protoChanged();
  emit visuallyChanged();
  emit weightChanged();
}

void BlendModel::setName(const QString &name) {
  if (nameCache_ == name) return;

  nameCache_ = name;
  proto_->set_name(name.toStdString());
  emit protoChanged();
  emit nameChanged();
}

void BlendModel::addPre() {
  if (pre_ != nullptr) return;

  pre_ = modelFactory_->createTransformModel(this);
  connect(pre_, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);
  // All transform changes are visual.
  connect(pre_, &AbstractBaseModel::protoChanged, this,
          &BlendModel::visuallyChanged);
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
  pre_ = nullptr;
  proto_->clear_pre();
  emit protoChanged();
  emit visuallyChanged();
  emit preChanged();
}

void BlendModel::addPost() {
  if (post_ != nullptr) return;

  post_ = modelFactory_->createTransformModel(this);
  connect(post_, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);
  // All transform changes are visual.
  connect(post_, &AbstractBaseModel::protoChanged, this,
          &BlendModel::visuallyChanged);
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
  post_ = nullptr;
  proto_->clear_post();
  emit protoChanged();
  emit visuallyChanged();
  emit postChanged();
}

void BlendModel::addFormula(const QString &type) {
  auto *model = modelFactory_->createFormulaModel(formulas_);
  connect(model, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);
  // All formula proto changes are visual.
  connect(model, &AbstractBaseModel::protoChanged, this,
          &BlendModel::visuallyChanged);

  Formula *formula = proto_->add_formulas();
  formula->set_type(type.toStdString());
  auto *w = formula->mutable_weight();
  w->set_x(1.f);
  w->set_y(1.f);
  model->setProto(formula);

  formulas_->append(model);
  emit visuallyChanged();
  emit protoChanged();
}

void BlendModel::deleteFormulaAt(int index) {
  if (index < 0 || index >= formulas_->size()) return;

  formulas_->remove(index);
  proto_->mutable_formulas()->DeleteSubrange(index, 1);
  emit visuallyChanged();
  emit protoChanged();
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
