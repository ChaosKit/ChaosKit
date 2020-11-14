#include <QImage>
#include <iostream>
#include "core/Color.h"
#include "core/ColorMapRegistry.h"
#include "core/SimpleHistogramGenerator.h"
#include "core/transforms.h"
#include "flame/Blend.h"
#include "flame/Formula.h"
#include "flame/System.h"
#include "flame/toSource.h"
#include "library/coloring_methods/Distance.h"
#include "library/formulas/DeJong.h"

using chaoskit::core::Blend;
using chaoskit::core::Color;
using chaoskit::core::ColorMapRegistry;
using chaoskit::core::FinalBlend;
using chaoskit::core::Formula;
using chaoskit::core::scale;
using chaoskit::core::SimpleHistogramGenerator;
using chaoskit::core::System;
using chaoskit::core::toSource;
using chaoskit::core::translate;
using chaoskit::library::DeJong;
using chaoskit::library::coloring_methods::Distance;

#define CLAMP(X, L, U) std::min(std::max((X), (L)), (U))

class LolRenderer : public chaoskit::core::Renderer {
 public:
  void updateHistogramBuffer(
      const chaoskit::core::HistogramBuffer &buffer) override {
    buffer_ = buffer.data();
  }

  QImage render() {
    QImage image(512, 512, QImage::Format_RGB32);
    for (int y = 0; y < 512; y++) {
      for (int x = 0; x < 512; x++) {
        int index = y * 512 + x;
        const Color &color = buffer_[index];
        double intensity = color.a * M_E;
        double logScale = std::log10(intensity + 1) / intensity;
        double scaledIntensity = color.a * logScale;
        double scale =
            logScale * std::pow(scaledIntensity, 2.2) / scaledIntensity;

        image.setPixelColor(x, y,
                            QColor::fromRgbF(CLAMP(color.r * scale, 0.0, 1.0),
                                             CLAMP(color.g * scale, 0.0, 1.0),
                                             CLAMP(color.b * scale, 0.0, 1.0)));
      }
    }
    return image;
  }

 private:
  const Color *buffer_ = nullptr;
};

int main(int argc, char **argv) {
  auto finalBlend = std::make_unique<FinalBlend>();
  finalBlend->post = scale(.5f, 1.f) * translate(.5f, .5f);

  auto formula = std::make_unique<Formula>();
  formula->source = DeJong().source();
  formula->params = {9.379666578024626e-01f, 1.938709271140397e+00f,
                     -1.580897020176053e-01f, -1.430070123635232e+00f};

  auto blend = std::make_unique<Blend>();
  blend->formulas.push_back(formula.get());
  blend->coloringMethod.source = Distance().source();
  blend->coloringMethod.params = {0.2f};

  auto system = std::make_unique<System>();
  system->blends.push_back(blend.get());
  system->finalBlend = finalBlend.get();
  std::cout << toSource(*system) << std::endl;

  ColorMapRegistry colorMaps;
  SimpleHistogramGenerator generator(*system, 512, 512);
  generator.setColorMap(colorMaps.get("Rainbow"));
  generator.setIterationCount(1000000);
  generator.setEnabled(true);

  LolRenderer renderer;
  generator.synchronizeResult(&renderer);
  QImage image = renderer.render();
  generator.finalizeStep(&renderer);

  image.save("lol.png");

  return 0;
}
