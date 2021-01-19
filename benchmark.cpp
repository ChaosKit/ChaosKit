#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include "ast/helpers.h"
#include "core/SimpleInterpreter.h"
#include "core/SystemParticle.h"
#include "core/SystemProcessor.h"
#include "core/ThreadLocalRng.h"
#include "core/TransformSystem.h"
#include "flame/ManagedDocument.h"
#include "flame/transforms.h"

void handwritten(int iterations) {
  using namespace chaoskit::core;
  ThreadLocalRng rng;

  SystemParticle particle;
  particle.particle.point =
      Point(rng.randomFloat(-2, 2), rng.randomFloat(-2, 2));
  particle.particle.color = rng.randomFloat(0, 1);
  particle.particle.skip = 0;
  particle.particle.ttl = rng.randomInt(1, 30);

  while (iterations--) {
    if (particle.particle.ttl == 0) {
      particle.particle.point =
          Point(rng.randomFloat(-2, 2), rng.randomFloat(-2, 2));
      particle.particle.color = rng.randomFloat(0, 1);
      particle.particle.skip = 0;
      particle.particle.ttl = 30;
    }

    Point output{sinf(9.379666578024626e-01f * particle.particle.y()) -
                     cosf(1.938709271140397e+00f * particle.particle.x()),
                 sinf(-1.580897020176053e-01f * particle.particle.x()) -
                     cosf(-1.430070123635232e+00f * particle.particle.y())};

    auto dx = particle.particle.x() - output.x();
    auto dy = particle.particle.y() - output.y();
    auto distance = sqrtf(dx * dx + dy * dy);
    particle.particle.point = output;
    particle.particle.color = modff(distance * .2f, &dx);
    --particle.particle.ttl;
  }
}

void interpreted(int iterations) {
  using namespace chaoskit;
  auto rng = std::make_shared<core::ThreadLocalRng>();

  auto transform = ([]() -> ast::Transform {
    using namespace chaoskit::ast::helpers;
    InputHelper input;
    OutputHelper output;
    ParameterHelper params;

    auto dx = input.x() - output.x();
    auto dy = input.y() - output.y();
    auto distance = sqrt(dx * dx + dy * dy);

    return {
        ast::Formula{sin(params[0] * input.y()) - cos(params[1] * input.x()),
                     sin(params[2] * input.x()) - cos(params[3] * input.y())},
        frac(distance * params[4])};
  })();
  core::TransformParams params{
      {core::TransformIndex(),
       {9.379666578024626e-01f, 1.938709271140397e+00f, -1.580897020176053e-01f,
        -1.430070123635232e+00f, .2f}}};

  core::SystemProcessor processor({transform, params}, rng);
  processor.setBounds({-2.f, 2.f, -2.f, 2.f});
  processor.setParticleLifetime(30);

  auto particle = processor.createParticle();
  while (iterations--) {
    particle = processor.process(particle);
  }
}

void oldFlame(int iterations) {
  using namespace chaoskit;
  using namespace chaoskit::ast::helpers;
  InputHelper input;
  OutputHelper output;
  ParameterHelper params;

  auto formula = new flame::Formula();
  formula->source =
      ast::Formula{sin(params[0] * input.y()) - cos(params[1] * input.x()),
                   sin(params[2] * input.x()) - cos(params[3] * input.y())};
  formula->params = {9.379666578024626e-01f, 1.938709271140397e+00f,
                     -1.580897020176053e-01f, -1.430070123635232e+00f};

  auto blend = new flame::Blend();
  blend->formulas.push_back(formula);
  blend->coloringMethod =
      ([&input, &output, &params]() -> flame::ColoringMethod {
        flame::ColoringMethod coloringMethod;
        auto dx = input.x() - output.x();
        auto dy = input.y() - output.y();
        auto distance = sqrt(dx * dx + dy * dy);
        coloringMethod.source = frac(distance * params[0]);
        coloringMethod.params = {.2f};
        return coloringMethod;
      })();

  auto system = new flame::System();
  system->blends.push_back(blend);
  system->ttl = 30;
  system->initialTransform = flame::scale(0.5);

  core::SimpleInterpreter interpreter(*system);
  auto particle = interpreter.randomizeParticle();
  while (iterations--) {
    particle = interpreter.processBlends(particle);
  }

  delete system;
  delete blend;
  delete formula;
}

std::chrono::milliseconds measure(void (*fn)(int), int iterations) {
  auto start = std::chrono::high_resolution_clock::now();
  fn(iterations);
  auto end = std::chrono::high_resolution_clock::now();

  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

int main(int argc, char** argv) {
  constexpr int iterations = 1000000;

  std::cout << "Handwritten implementation: "
            << measure(handwritten, iterations).count() << "ms" << std::endl;

  std::cout << "Using the old flame-centric code: "
            << measure(oldFlame, iterations).count() << "ms" << std::endl;

  std::cout << "Using the interpreter: "
            << measure(interpreted, iterations).count() << "ms" << std::endl;

  return 0;
}
