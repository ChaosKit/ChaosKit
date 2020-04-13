#include "DocumentAdopter.h"

namespace chaoskit::ui {

using chaoskit::state::Id;

void DocumentAdopter::visit(core::Blend& blend) {
  Id systemId = idStack_.back();
  Id blendId = store_.associateNewChildWith<core::System, core::Blend>(
      systemId, [&blend](core::Blend* target) {
        target->enabled = blend.enabled;
        target->pre = blend.pre;
        target->post = blend.post;
        target->weight = blend.weight;
        target->name = blend.name;
        target->coloringMethod = std::move(blend.coloringMethod);
      });

  idStack_.push_back(blendId);
  for (auto* formula : blend.formulas) {
    visit(*formula);
  }
  idStack_.pop_back();
}

void DocumentAdopter::visit(core::Document& document) {
  Id documentId =
      store_.create<core::Document>([&document](core::Document* target) {
        target->gamma = document.gamma;
        target->exposure = document.exposure;
        target->vibrancy = document.vibrancy;
        target->colorMap = document.colorMap;
        target->width = document.width;
        target->height = document.height;
      });

  idStack_.push_back(documentId);
  visit(*document.system);
  idStack_.pop_back();
}

void DocumentAdopter::visit(core::FinalBlend& blend) {
  Id systemId = idStack_.back();
  Id blendId = store_.associateNewChildWith<core::System, core::FinalBlend>(
      systemId, [&blend](core::FinalBlend* target) {
        target->enabled = blend.enabled;
        target->pre = blend.pre;
        target->post = blend.post;
        target->coloringMethod = std::move(blend.coloringMethod);
      });

  idStack_.push_back(blendId);
  for (auto* formula : blend.formulas) {
    visit(*formula);
  }
  idStack_.pop_back();
}

void DocumentAdopter::visit(core::Formula& formula) {
  Id blendId = idStack_.back();
  auto formulaUpdater = [&formula](core::Formula* target) {
    *target = std::move(formula);
  };

  if (DocumentStore::matchesType<core::Blend>(blendId)) {
    store_.associateNewChildWith<core::Blend, core::Formula>(blendId,
                                                             formulaUpdater);
  } else {
    store_.associateNewChildWith<core::FinalBlend, core::Formula>(
        blendId, formulaUpdater);
  }
}

void DocumentAdopter::visit(core::System& system) {
  Id documentId = idStack_.back();
  Id systemId = store_.associateNewChildWith<core::Document, core::System>(
      documentId, [&system](core::System* target) {
        target->ttl = system.ttl;
        target->initialTransform = system.initialTransform;
      });

  idStack_.push_back(systemId);
  for (auto* blend : system.blends) {
    visit(*blend);
  }
  if (system.finalBlend) {
    visit(*system.finalBlend);
  }
  idStack_.pop_back();
}

}  // namespace chaoskit::ui
