#include "ColorMapModel.h"

namespace chaoskit::ui {

const QString& ColorMapModel::name() {
  ensureNameCacheIsFilled();
  return *nameCache_;
}

void ColorMapModel::setName(const QString& name) {
  ensureNameCacheIsFilled();
  if (*nameCache_ == name) return;

  *nameCache_ = name;
  proto_->set_name(name.toStdString());
  emit nameChanged();
}

void ColorMapModel::ensureNameCacheIsFilled() {
  if (nameCache_) return;
  nameCache_ = QString::fromStdString(proto_->name());
}

}  // namespace chaoskit::ui
