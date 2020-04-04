#include "ColorMapRegistry.h"
#include <algorithm>

namespace chaoskit::ui {

ColorMapRegistry::ColorMapRegistry(QObject* parent)
    : QObject(parent), registry_(), nameCache_() {
  const auto& names = registry_.names();
  std::transform(names.begin(), names.end(), std::back_inserter(nameCache_),
                 &QString::fromStdString);
}

void ColorMapRegistry::add(const QString& name,
                           std::unique_ptr<core::ColorMap> colorMap) {
  registry_.add(name.toStdString(), std::move(colorMap));

  nameCache_.append(name);
  std::stable_sort(nameCache_.begin(), nameCache_.end());
  emit namesChanged();
}

const core::ColorMap* ColorMapRegistry::get(const QString& name) const {
  return registry_.get(name.toStdString());
}

const QString& ColorMapRegistry::defaultName() const {
  return nameCache_.front();
}

}  // namespace chaoskit::ui
