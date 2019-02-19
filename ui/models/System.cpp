#include "System.h"

using chaoskit::core::Params;
using chaoskit::core::SystemIndex;

namespace chaoskit {
namespace ui {

System::System(QObject *parent) : QObject(parent) {
  final_blend_ = new Blend(this);
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
  auto *blend = new Blend(this);
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

void System::removeBlend(Blend *blend) {
  blend->disconnect(this);
  blends_.removeOne(blend);
  emit sourceChanged();
}

void System::removeBlendAt(int index) {
  blends_.at(index)->disconnect(this);
  blends_.remove(index);
  emit sourceChanged();
}

Params System::params() const {
  Params params;

  for (size_t i = 0; i < blends_.size(); ++i) {
    const Blend *blend = blends_[i];
    for (size_t j = 0; j < blend->formulaCount(); ++j) {
      const auto &formulaParams =
          blend->formulaAt(static_cast<int>(j))->params();
      if (!formulaParams.empty()) {
        params[SystemIndex{i, j}] = formulaParams.toStdVector();
      }
    }
  }

  if (!final_blend_) {
    return params;
  }

  for (size_t j = 0; j < final_blend_->formulaCount(); ++j) {
    const auto &formulaParams =
        final_blend_->formulaAt(static_cast<int>(j))->params();
    if (!formulaParams.empty()) {
      params[SystemIndex{SystemIndex::FINAL_BLEND, j}] =
          formulaParams.toStdVector();
    }
  }
  return params;
}

}  // namespace ui
}  // namespace chaoskit
