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

  return *settings_;
}

}  // namespace chaoskit::ui
