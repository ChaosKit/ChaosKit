#include "ColorMapModel.h"

namespace chaoskit::ui {

ColorMapModel::ColorMapModel(QObject* parent) : BaseModel<ColorMap>(parent) {
  setObjectName("colorMap");
  // Emit indexChanged every time the name changes.
  connect(this, &ColorMapModel::nameChanged, this,
          &ColorMapModel::indexChanged);
}

void ColorMapModel::setProto(ColorMap* proto) {
  BaseModel::setProto(proto);

  updateNameCache();

  emit protoChanged();
}

void ColorMapModel::setName(const QString& name) {
  if (*nameCache_ == name) return;

  // Change the color map itself. If the desired map doesn't exist, fail
  // gracefully and reject the change.
  //
  // You could ask why this doesn't reuse updateNameCache(). This would be
  // perfectly appropriate if we were updating the proto directly and propagate
  // down to derived fields (nameCache and colorMap).
  //
  // However, while normally the proto would be the source of truth, here we're
  // really replacing nameCache and propagate the change up to the proto because
  // it allows us to avoid a back-and-forth conversion between QString and
  // std::string. Also, a separate code path allows us to better react to errors
  // with signals.
  Q_ASSERT(colorMapRegistry_ != nullptr);
  if (!colorMapRegistry_->names().contains(name)) {
    emit invalidNamePicked();
    return;
  }
  colorMap_ = colorMapRegistry_->get(name);

  // Update nameCache and the proto.
  *nameCache_ = name;
  proto_->set_name(name.toStdString());
  emit protoChanged();
  emit nameChanged();
}

void ColorMapModel::setColorMapRegistry(ColorMapRegistry* registry) {
  if (colorMapRegistry_ == registry) return;

  colorMapRegistry_ = registry;
  updateColorMap();
  emit registryChanged();
}

int ColorMapModel::index() const {
  if (nameCache_ && colorMapRegistry_ != nullptr) {
    return colorMapRegistry_->names().indexOf(*nameCache_);
  }
  return -1;
}

void ColorMapModel::updateNameCache() {
  QString nameCache = QString::fromStdString(proto_->name());
  if (nameCache.isEmpty() && colorMapRegistry_ != nullptr) {
    nameCache = colorMapRegistry_->defaultName();
  }
  if (nameCache_ == nameCache) return;

  nameCache_ = nameCache;
  updateColorMap();
  emit nameChanged();
}

void ColorMapModel::updateColorMap() {
  if (nameCache_ && colorMapRegistry_ != nullptr &&
      colorMapRegistry_->names().contains(*nameCache_)) {
    colorMap_ = colorMapRegistry_->get(*nameCache_);
  } else {
    colorMap_ = nullptr;
  }
}

QColor ColorMapModel::colorAt(float position) const {
  if (colorMap_ == nullptr) {
    return QColor(Qt::white);
  }
  core::Color color = colorMap_->map(std::clamp(position, 0.f, 1.f));
  return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

}  // namespace chaoskit::ui
