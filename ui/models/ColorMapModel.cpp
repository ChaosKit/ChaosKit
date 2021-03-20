#include "ColorMapModel.h"

namespace chaoskit::ui {

void ColorMapModel::setProto(ColorMap* proto) {
  BaseModel::setProto(proto);
  updateNameCache();
}

void ColorMapModel::setName(const QString& name) {
  if (*nameCache_ == name) return;

  Q_ASSERT(colorMapRegistry_ != nullptr);
  if (!colorMapRegistry_->names().contains(name)) {
    emit invalidNamePicked();
    return;
  }
  colorMap_ = colorMapRegistry_->get(name);

  *nameCache_ = name;
  proto_->set_name(name.toStdString());
  emit nameChanged();
}

void ColorMapModel::setColorMapRegistry(ColorMapRegistry* registry) {
  if (colorMapRegistry_ == registry) return;

  colorMapRegistry_ = registry;
  updateColorMap();
}

void ColorMapModel::updateNameCache() {
  nameCache_ = QString::fromStdString(proto_->name());
  updateColorMap();
}

void ColorMapModel::updateColorMap() {
  if (nameCache_ && colorMapRegistry_ != nullptr &&
      colorMapRegistry_->names().contains(*nameCache_)) {
    colorMap_ = colorMapRegistry_->get(*nameCache_);
  } else {
    colorMap_ = nullptr;
  }
}

}  // namespace chaoskit::ui
