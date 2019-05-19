#include <iostream>
#include "core/models/DocumentModel.h"
#include "core/models/SystemModel.h"
#include "core/structures/System.h"
#include "core/toSource.h"
#include "core/transforms.h"
#include "library/DeJong.h"

using namespace chaoskit::core;

int main(int argc, char** argv) {
  DocumentModel document;
  auto system = document.system();
  system.finalBlend().setPost(translate(.5f, .5f) * scale(.5f, 1.f));

  auto formula = system.blends().add().formulas().add();
  formula.setSource(chaoskit::library::DeJong().source());
  formula.setParams({9.379666578024626e-01f, 1.938709271140397e+00f,
                     -1.580897020176053e-01f, -1.430070123635232e+00f});

  std::cout << toSource(system) << std::endl;

  return 0;
}
