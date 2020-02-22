#ifndef CHAOSKIT_CORE_COLORMAPREGISTRY_H
#define CHAOSKIT_CORE_COLORMAPREGISTRY_H

#include <set>
#include <string>
#include <unordered_map>
#include "ColorMap.h"

namespace chaoskit::core {

class ColorMapRegistry {
 public:
  ColorMapRegistry();
  void add(std::string name, std::unique_ptr<ColorMap> colorMap);
  [[nodiscard]] const ColorMap* get(const std::string& name) const;
  [[nodiscard]] const std::set<std::string>& names() const { return names_; }

 private:
  std::unordered_map<std::string, std::unique_ptr<ColorMap>> colorMaps_;
  std::set<std::string> names_;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_COLORMAPREGISTRY_H
