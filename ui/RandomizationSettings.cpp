#include "RandomizationSettings.h"

namespace chaoskit::ui {

RandomizationSettings::Builder::Builder()
    : settings_(new RandomizationSettings) {}

RandomizationSettings RandomizationSettings::Builder::build() {
  // Empty set means all formula types
  if (settings_->allowedFormulaTypes_.isEmpty()) {
    for (const auto& type : library::FormulaType::_values()) {
      if (type == +library::FormulaType::Invalid) continue;

      settings_->allowedFormulaTypes_.append(type);
    }
  }

  // Empty set means all coloring methods
  if (settings_->allowedColoringMethodsInBlend_.isEmpty()) {
    for (const auto& type : library::ColoringMethodType::_values()) {
      settings_->allowedColoringMethodsInBlend_.append(type);
    }
  }
  if (settings_->allowedColoringMethodsInFinalBlend_.isEmpty()) {
    for (const auto& type : library::ColoringMethodType::_values()) {
      settings_->allowedColoringMethodsInFinalBlend_.append(type);
    }
  }

  return *settings_;
}

}  // namespace chaoskit::ui
