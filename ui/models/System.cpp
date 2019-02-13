#include "System.h"

namespace chaoskit {
namespace ui {
namespace models {

using chaoskit::core::Params;
using chaoskit::core::SystemIndex;

Blend *System::addBlend() {
  auto *blend = new Blend;
  addBlend(blend);
  return blend;
}

void System::addBlend(Blend *blend) {
  blends_.append(blend);
  emit blendsChanged();
}

void System::setFinalBlend(Blend *blend) {
  final_blend_ = blend;
  emit finalBlendChanged();
}

Params System::params() const {
  Params params;

  for (size_t i = 0; i < blends_.size(); ++i) {
    const auto &formulas = blends_[i]->formulas();
    for (size_t j = 0; j < formulas.size(); ++j) {
      const auto &formulaParams = formulas[j]->params();
      if (!formulaParams.empty()) {
        params[SystemIndex{i, j}] = formulaParams.toStdVector();
      }
    }
  }

  if (!final_blend_) {
    return params;
  }

  for (size_t j = 0; j < final_blend_->formulas().size(); ++j) {
    const auto &formulaParams = final_blend_->formulas()[j]->params();
    if (!formulaParams.empty()) {
      params[SystemIndex{SystemIndex::FINAL_BLEND, j}] =
          formulaParams.toStdVector();
    }
  }
  return params;
}

}  // namespace models
}  // namespace ui
}  // namespace chaoskit
