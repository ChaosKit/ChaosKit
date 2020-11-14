#include "DocumentAdopter.h"

namespace chaoskit::ui {

using chaoskit::state::Id;

void DocumentAdopter::visit(flame::Blend& blend) {
  Id systemId = idStack_.back();
  Id blendId = store_.associateNewChildWith<flame::System, flame::Blend>(
      systemId, [&blend](flame::Blend* target) {
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

void DocumentAdopter::visit(flame::Document& document) {
  Id documentId =
      store_.create<flame::Document>([&document](flame::Document* target) {
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

void DocumentAdopter::visit(flame::FinalBlend& blend) {
  Id systemId = idStack_.back();
  Id blendId = store_.associateNewChildWith<flame::System, flame::FinalBlend>(
      systemId, [&blend](flame::FinalBlend* target) {
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

void DocumentAdopter::visit(flame::Formula& formula) {
  Id blendId = idStack_.back();
  auto formulaUpdater = [&formula](flame::Formula* target) {
    *target = std::move(formula);
  };

  if (DocumentStore::matchesType<flame::Blend>(blendId)) {
    store_.associateNewChildWith<flame::Blend, flame::Formula>(blendId,
                                                               formulaUpdater);
  } else {
    store_.associateNewChildWith<flame::FinalBlend, flame::Formula>(
        blendId, formulaUpdater);
  }
}

void DocumentAdopter::visit(flame::System& system) {
  Id documentId = idStack_.back();
  Id systemId = store_.associateNewChildWith<flame::Document, flame::System>(
      documentId, [&system](flame::System* target) {
        target->skip = system.skip;
        target->ttl = system.ttl;
        target->initialTransform = system.initialTransform;
      });

  idStack_.push_back(systemId);
  for (auto* blend : system.blends) {
    visit(*blend);

    if (blend == system.isolatedBlend) {
      store_.update<flame::System>(systemId, [](flame::System* target) {
        target->isolatedBlend = target->blends.back();
      });
    }
  }
  if (system.finalBlend) {
    visit(*system.finalBlend);
  }
  idStack_.pop_back();
}

}  // namespace chaoskit::ui
