#include "ColorMapRegistry.h"
#include "BlackWhiteColorMap.h"
#include "PaletteColorMap.h"
#include "RainbowColorMap.h"
#include "errors.h"

namespace chaoskit::core {

ColorMapRegistry::ColorMapRegistry() : colorMaps_() {
  add("BGR", std::make_unique<PaletteColorMap>(std::vector<Color>{
                 Color{0.f, 0.f, 1.f},
                 Color{0.f, 1.f, 0.f},
                 Color{1.f, 0.f, 0.f},
             }));
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

const std::string& ColorMapRegistry::defaultName() const {
  static const std::string empty;
  if (names_.empty()) {
    return empty;
  }
  return *names_.begin();
}

}  // namespace chaoskit::core
