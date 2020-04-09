#include "RandomizationSettings.h"
#include <magic_enum.hpp>

namespace chaoskit::ui {

RandomizationSettings::Builder::Builder()
    : settings_(new RandomizationSettings) {}

RandomizationSettings RandomizationSettings::Builder::build() {
  // Empty set means all formula types
  if (settings_->allowedFormulaTypes_.isEmpty()) {
    for (const auto& type : magic_enum::enum_values<library::FormulaType>()) {
      if (type == library::FormulaType::Invalid) continue;

      settings_->allowedFormulaTypes_.append(type);
    }
  }

  // Empty set means all coloring methods
  if (settings_->allowedColoringMethodsInBlend_.isEmpty()) {
    for (const auto& type :
         magic_enum::enum_values<library::ColoringMethodType>()) {
      settings_->allowedColoringMethodsInBlend_.append(type);
    }
  }
  if (settings_->allowedColoringMethodsInFinalBlend_.isEmpty()) {
    for (const auto& type :
         magic_enum::enum_values<library::ColoringMethodType>()) {
      settings_->allowedColoringMethodsInFinalBlend_.append(type);
    }
  }

  return *settings_;
}

}  // namespace chaoskit::ui
