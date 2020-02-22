#include "ColorMapRegistry.h"

namespace chaoskit::ui {

ColorMapRegistry::ColorMapRegistry(QObject* parent)
    : QObject(parent), registry_() {}

void ColorMapRegistry::add(const QString& name,
                           std::unique_ptr<core::ColorMap> colorMap) {
  registry_.add(name.toStdString(), std::move(colorMap));
}

const core::ColorMap* ColorMapRegistry::get(const QString& name) const {
  return registry_.get(name.toStdString());
}

}  // namespace chaoskit::ui
