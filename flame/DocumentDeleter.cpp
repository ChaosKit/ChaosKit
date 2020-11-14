#include "DocumentDeleter.h"
#include "Blend.h"
#include "Document.h"
#include "System.h"

namespace chaoskit::flame {

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

}  // namespace chaoskit::flame
