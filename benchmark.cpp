#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include "ast/helpers.h"
#include "core/SystemParticle.h"
#include "core/SystemProcessor.h"
#include "core/ThreadLocalRng.h"
#include "core/TransformSystem.h"

class Implementation {
 public:
  virtual void setUp() {}
  virtual void tearDown() {}
  virtual void run(int iterations) = 0;
};

class LambdaImplementation : public Implementation {
  void (*fn_)(int);

 public:
  explicit LambdaImplementation(void (*fn)(int)) : fn_(fn) {}

  void run(int iterations) override { fn_(iterations); }
};

void handwritten(int iterations) {
  using namespace chaoskit::core;
  ThreadLocalRng rng;

  SystemParticle particle;
  particle.point = Point(rng.randomFloat(-2, 2), rng.randomFloat(-2, 2));
  particle.color = rng.randomFloat(0, 1);
  particle.skip = 0;
  particle.ttl = rng.randomInt(1, 30);

  while (iterations--) {
    if (particle.ttl == 0) {
      particle.point = Point(rng.randomFloat(-2, 2), rng.randomFloat(-2, 2));
      particle.color = rng.randomFloat(0, 1);
      particle.skip = 0;
      particle.ttl = 30;
    }

    Point output{sinf(9.379666578024626e-01f * particle.y()) -
                     cosf(1.938709271140397e+00f * particle.x()),
                 sinf(-1.580897020176053e-01f * particle.x()) -
                     cosf(-1.430070123635232e+00f * particle.y())};

    auto dx = particle.x() - output.x();
    auto dy = particle.y() - output.y();
    auto distance = sqrtf(dx * dx + dy * dy);
    particle.point = output;
    particle.color = modff(distance * .2f, &dx);
    --particle.ttl;
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

std::chrono::milliseconds measure(Implementation* impl, int iterations) {
  impl->setUp();
  auto start = std::chrono::high_resolution_clock::now();
  impl->run(iterations);
  auto end = std::chrono::high_resolution_clock::now();
  impl->tearDown();

  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

std::chrono::milliseconds measure(void (*fn)(int), int iterations) {
  LambdaImplementation impl(fn);
  return measure(&impl, iterations);
}

int main(int argc, char** argv) {
  constexpr int iterations = 1000000;

  std::cout << "Handwritten implementation: "
            << measure(handwritten, iterations).count() << "ms" << std::endl;

  std::cout << "Using the interpreter: "
            << measure(interpreted, iterations).count() << "ms" << std::endl;

  return 0;
}
