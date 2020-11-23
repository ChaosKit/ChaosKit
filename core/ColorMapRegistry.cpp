#include "ColorMapRegistry.h"
#include "BlackWhiteColorMap.h"
#include "RainbowColorMap.h"
#include "errors.h"

namespace chaoskit::core {

ColorMapRegistry::ColorMapRegistry() : colorMaps_() {
  add("BlackWhite", std::make_unique<BlackWhiteColorMap>());
  add("Rainbow", std::make_unique<RainbowColorMap>());
}

void ColorMapRegistry::add(std::string name,
                           std::unique_ptr<ColorMap> colorMap) {
  auto [_, ok] = names_.insert(name);
  if (ok) {
    colorMaps_.emplace(std::move(name), std::move(colorMap));
  }
}

const ColorMap* ColorMapRegistry::get(const std::string& name) const {
  try {
    return colorMaps_.at(name).get();
  } catch (std::out_of_range&) {
    throw InvalidColorMap(name);
  }
}

}  // namespace chaoskit::core
