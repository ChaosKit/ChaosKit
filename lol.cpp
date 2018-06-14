#include <iostream>
#include <string>
#include <vector>
#include <QImage>
#include <cmath>
#include <core/ThreadLocalRng.h>

#include "ast/ast.h"
#include "ast/helpers.h"
#include "library/DeJong.h"
#include "core/Point.h"
#include "core/SimpleInterpreter.h"

using chaoskit::ast::Blend;
using chaoskit::ast::Formula;
using chaoskit::ast::Transform;
using chaoskit::ast::System;
using chaoskit::ast::WeightedFormula;
using chaoskit::ast::LimitedBlend;
using chaoskit::core::Point;
using chaoskit::core::SimpleInterpreter;
using chaoskit::core::ThreadLocalRng;
using chaoskit::library::DeJong;
using namespace chaoskit::ast::helpers;

#define CLAMP(X, L, U) std::min(std::max((X), (L)), (U))

int main() {
  auto rng = std::make_shared<ThreadLocalRng>();

  Formula formula = DeJong().source();
  Blend final({}, Transform::identity(), Transform(1, 0, 0.5, 0, 2, 1));
  System system({LimitedBlend(Blend{formula}, 1.0)}, final);

  Point point{rng->randomFloat(-1.f, 1.f), rng->randomFloat(-1.f, 1.f)};

  SimpleInterpreter interpreter(system, {9.379666578024626e-01f, 1.938709271140397e+00f, -1.580897020176053e-01f, -1.430070123635232e+00f}, rng);

  std::vector<int> buffer(512*512);

  for (int i = 0; i < 1000000; i++) {
    auto [next_state, output] = interpreter(point);
    point = next_state;
    int x = CLAMP(static_cast<int>(output.x() * 128.f + 256.f), 0, 511);
    int y = CLAMP(static_cast<int>(output.y() * 128.f + 256.f), 0, 511);

    buffer[y*512+x]++;
  }

  std::transform(buffer.begin(), buffer.end(), buffer.begin(), [](int pixel) {
    return CLAMP(static_cast<int>(std::log10(pixel + 1.f) * 64), 0, 255);
  });

  QImage image(512, 512, QImage::Format_Grayscale8);
  for (int y = 0; y < 512; y++) {
    for (int x = 0; x < 512; x++) {
      int color = buffer[y * 512 + x];
      image.setPixelColor(x, y, QColor(color, color, color));
    }
  }
  image.save("lol.png");

  return 0;
}
