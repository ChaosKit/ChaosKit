#include "System.h"

using chaoskit::core::Params;
using chaoskit::core::SystemIndex;

namespace chaoskit {
namespace ui {

System::System(QObject *parent) : QObject(parent) {
  final_blend_ = new Blend();
  final_blend_->setName("Final Blend");

  connect(final_blend_, &Blend::preChanged, this,
          &System::finalBlendParamsChanged);
  connect(final_blend_, &Blend::postChanged, this,
          &System::finalBlendParamsChanged);
  connect(final_blend_, &Blend::sourceChanged, this,
          &System::finalBlendSourceChanged);
  connect(final_blend_, &Blend::paramsChanged, this,
          &System::finalBlendParamsChanged);
}

Blend *System::addBlend() {
  auto *blend = new Blend();
  addBlend(blend);
  return blend;
}

void System::addBlend(Blend *blend) {
  blends_.append(blend);
  emit sourceChanged();

  connect(blend, &Blend::preChanged, this, &System::paramsChanged);
  connect(blend, &Blend::postChanged, this, &System::paramsChanged);
  connect(blend, &Blend::sourceChanged, this, &System::sourceChanged);
  connect(blend, &Blend::paramsChanged, this, &System::paramsChanged);
  connect(blend, &Blend::weightChanged, this, &System::paramsChanged);
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

}  // namespace ui
}  // namespace chaoskit
