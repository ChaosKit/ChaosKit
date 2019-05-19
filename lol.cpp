#include <QImage>
#include <iostream>
#include "core/SimpleHistogramGenerator.h"
#include "core/models/DocumentModel.h"
#include "core/models/SystemModel.h"
#include "core/structures/System.h"
#include "core/toSource.h"
#include "core/transforms.h"
#include "library/DeJong.h"

using chaoskit::core::DocumentModel;
using chaoskit::core::scale;
using chaoskit::core::SimpleHistogramGenerator;
using chaoskit::core::toSource;
using chaoskit::core::translate;
using chaoskit::library::DeJong;

#define CLAMP(X, L, U) std::min(std::max((X), (L)), (U))

int main(int argc, char** argv) {
  DocumentModel document;
  auto system = document.system();
  system.finalBlend().setPost(scale(.5f, 1.f) * translate(.5f, .5f));

  auto formula = system.blends().add().formulas().add();
  formula.setSource(chaoskit::library::DeJong().source());
  formula.setParams({9.379666578024626e-01f, 1.938709271140397e+00f,
                     -1.580897020176053e-01f, -1.430070123635232e+00f});

  std::cout << toSource(system) << std::endl;

  SimpleHistogramGenerator generator(system, 512, 512);
  generator.setIterationCount(1000000);
  generator.run();

  const float* buffer = generator.data();

  QImage image(512, 512, QImage::Format_Grayscale8);
  for (int y = 0; y < 512; y++) {
    for (int x = 0; x < 512; x++) {
      int index = y * 512 + x;
      int color =
          CLAMP(static_cast<int>(std::log10(buffer[index] + 1.f) * 64), 0, 255);
      image.setPixelColor(x, y, QColor(color, color, color));
    }
  }
  image.save("lol.png");

  return 0;
}
