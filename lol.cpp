#include <QImage>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "ast/ast.h"

#include "ast/helpers.h"
#include "core/Point.h"
#include "core/SimpleBlender.h"
#include "core/ThreadLocalRng.h"
#include "core/util.h"
#include "library/DeJong.h"

using chaoskit::core::Blend;
using chaoskit::core::Formula;
using chaoskit::core::Params;
using chaoskit::core::Point;
using chaoskit::core::SimpleBlender;
using chaoskit::core::System;
using chaoskit::core::Transform;
using chaoskit::core::ThreadLocalRng;
using chaoskit::core::formulaIndex;
using chaoskit::library::DeJong;
using namespace chaoskit::ast::helpers;

#define CLAMP(X, L, U) std::min(std::max((X), (L)), (U))

int main() {
  auto rng = std::make_shared<ThreadLocalRng>();

  Formula formula = Formula::build<DeJong>(
      {9.379666578024626e-01f, 1.938709271140397e+00f, -1.580897020176053e-01f,
       -1.430070123635232e+00f});
  Blend final{Transform::identity(),
              Transform::fromScale(0.5f, 1.f).translate(0.5f, 0.5f)};
  Blend blend;
  blend.formulas.push_back(formula);

  System system{{blend}, final};

  SimpleBlender generator(system, 512, 512, rng);
  generator.setIterationCount(1000000);
  generator.run();

  const float *buffer = generator.data();

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
