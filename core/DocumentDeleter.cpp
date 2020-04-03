#include "DocumentDeleter.h"

namespace chaoskit::core {

void DocumentDeleter::visit(Blend& blend) {
  for (auto* formula : blend.formulas) {
    delete formula;
  }
}

void DocumentDeleter::visit(Document& document) {
  if (document.system) {
    visit(*document.system);
    delete document.system;
  }
}

void DocumentDeleter::visit(FinalBlend& blend) {
  for (auto* formula : blend.formulas) {
    delete formula;
  }
}

void DocumentDeleter::visit(System& system) {
  for (auto* blend : system.blends) {
    visit(*blend);
    delete blend;
  }
  if (system.finalBlend) {
    visit(*system.finalBlend);
    delete system.finalBlend;
  }
}

}  // namespace chaoskit::core
