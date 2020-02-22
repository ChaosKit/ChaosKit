#include <QImage>
#include <iostream>
#include "core/SimpleHistogramGenerator.h"
#include "core/structures/Blend.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "core/toSource.h"
#include "core/transforms.h"
#include "library/DeJong.h"

using chaoskit::core::Blend;
using chaoskit::core::FinalBlend;
using chaoskit::core::Formula;
using chaoskit::core::scale;
using chaoskit::core::SimpleHistogramGenerator;
using chaoskit::core::System;
using chaoskit::core::toSource;
using chaoskit::core::translate;
using chaoskit::library::DeJong;

#define CLAMP(X, L, U) std::min(std::max((X), (L)), (U))

int main(int argc, char **argv) {
  auto finalBlend = std::make_unique<FinalBlend>();
  finalBlend->post = scale(.5f, 1.f) * translate(.5f, .5f);

  auto formula = std::make_unique<Formula>();
  formula->source = DeJong().source();
  formula->params = {9.379666578024626e-01f, 1.938709271140397e+00f,
                     -1.580897020176053e-01f, -1.430070123635232e+00f};

  auto blend = std::make_unique<Blend>();
  blend->formulas.push_back(formula.get());

  auto system = std::make_unique<System>();
  system->blends.push_back(blend.get());
  system->finalBlend = finalBlend.get();
  std::cout << toSource(*system) << std::endl;

  SimpleHistogramGenerator generator(*system, 512, 512);
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
