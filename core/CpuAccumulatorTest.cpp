#include <gmock/gmock.h>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "CameraSystem.h"
#include "Color.h"
#include "ColorMap.h"
#include "ColorMapRegistry.h"
#include "CpuAccumulator.h"
#include "Lifetime.h"
#include "Rng.h"
#include "TransformSystem.h"
#include "ast/Formula.h"
#include "ast/Input.h"
#include "ast/Transform.h"
#include "ast/helpers.h"
#include "testing/MockRng.h"
#include "testing/StaticRng.h"

namespace chaoskit::core {
namespace {

using ::testing::_;
using ::testing::Return;

class SequenceFloatRng final : public Rng {
 public:
  explicit SequenceFloatRng(std::vector<float> floats, int intVal = 1)
      : floats_(std::move(floats)), intVal_(intVal) {}

  double randomDouble(double /*min*/, double /*max*/) override { return 0.0; }
  float randomFloat(float /*min*/, float /*max*/) override {
    if (floats_.empty()) return 0.0f;
    float v = floats_[idx_ % floats_.size()];
    ++idx_;
    return v;
  }
  int randomInt(int /*min*/, int /*max*/) override { return intVal_; }

 private:
  std::vector<float> floats_;
  size_t idx_ = 0;
  int intVal_;
};

class DeterministicPcgRng final : public Rng {
 public:
  explicit DeterministicPcgRng(uint64_t seed) : gen_(seed) {}

  double randomDouble(double min, double max) override {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen_);
  }
  float randomFloat(float min, float max) override {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen_);
  }
  int randomInt(int min, int max) override {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen_);
  }

 private:
  std::mt19937_64 gen_;
};

TEST(CpuAccumulatorTest, AccumulatesBatchWithColorMapAndTracksPlottedCount) {
  // Constant formula plotting at center (0, 0) with color coordinate 0.5.
  CameraSystem system = {
      TransformSystem{ast::Transform(ast::Formula{0.f, 0.f}, 0.5f), {}},
      stdx::nullopt,
      Lifetime(),
  };
  ColorMapRegistry registry;
  const ColorMap* bgr = registry.get("BGR");
  ASSERT_NE(bgr, nullptr);

  auto rng = std::make_shared<StaticRng>(0.0f);
  CpuAccumulator accumulator(system, 4, 4, bgr, rng);

  EXPECT_EQ(accumulator.plottedIterations(), 0u);
  EXPECT_EQ(accumulator.accumulate(5), 5u);
  EXPECT_EQ(accumulator.plottedIterations(), 5u);
  EXPECT_DOUBLE_EQ(accumulator.iterationsPerPixel(), 5.0 / 16.0);

  // Center of 4x4 maps (0, 0) -> fx = 2.0, fy = 2.0 -> pixel (2, 2).
  // BGR(0.5) = (0, 1, 0, 1), accumulated 5 times => (0, 5, 0, 5).
  const Color* hit = accumulator.histogram()(2, 2);
  EXPECT_FLOAT_EQ(hit->r, 0.f);
  EXPECT_FLOAT_EQ(hit->g, 5.f);
  EXPECT_FLOAT_EQ(hit->b, 0.f);
  EXPECT_FLOAT_EQ(hit->a, 5.f);

  const Color* empty = accumulator.histogram()(0, 0);
  EXPECT_FLOAT_EQ(empty->a, 0.f);
}

TEST(CpuAccumulatorTest, MapsZoomToFillInLandscapeAndPortrait) {
  ColorMapRegistry registry;
  const ColorMap* bw = registry.get("BlackWhite");
  ASSERT_NE(bw, nullptr);

  // Point at (0.5, 0.25) with color 1.0:
  // Landscape 200x100: halfMax = 100, halfW = 100, halfH = 50 -> (150, 75).
  CameraSystem system = {
      TransformSystem{ast::Transform(ast::Formula{0.5f, 0.25f}, 1.0f), {}},
      stdx::nullopt,
      Lifetime(),
  };
  auto rng = std::make_shared<StaticRng>(0.0f);
  CpuAccumulator landscape(system, 200, 100, bw, rng);
  EXPECT_EQ(landscape.accumulate(1), 1u);
  EXPECT_FLOAT_EQ(landscape.histogram()(150, 75)->a, 1.f);

  // Portrait 100x200: halfMax = 100, halfW = 50, halfH = 100.
  // Point (0.25, 0.5) -> (75, 150).
  CameraSystem portraitSystem = {
      TransformSystem{ast::Transform(ast::Formula{0.25f, 0.5f}, 1.0f), {}},
      stdx::nullopt,
      Lifetime(),
  };
  CpuAccumulator portrait(portraitSystem, 100, 200, bw, rng);
  EXPECT_EQ(portrait.accumulate(1), 1u);
  EXPECT_FLOAT_EQ(portrait.histogram()(75, 150)->a, 1.f);
}

TEST(CpuAccumulatorTest, CountsOutOfBoundsFiniteStepsInPlottedIterations) {
  ColorMapRegistry registry;
  const ColorMap* bw = registry.get("BlackWhite");

  CameraSystem offscreen = {
      TransformSystem{ast::Transform(ast::Formula{2.5f, -3.0f}, 1.0f), {}},
      stdx::nullopt,
      Lifetime(),
  };
  auto rng = std::make_shared<StaticRng>(0.0f);
  CpuAccumulator accumulator(offscreen, 8, 8, bw, rng);
  EXPECT_EQ(accumulator.accumulate(4), 4u);
  EXPECT_EQ(accumulator.plottedIterations(), 4u);
  EXPECT_DOUBLE_EQ(accumulator.iterationsPerPixel(), 4.0 / 64.0);

  for (size_t i = 0; i < accumulator.histogram().size(); ++i) {
    EXPECT_FLOAT_EQ(accumulator.histogram().data()[i].a, 0.f);
  }
}

TEST(CpuAccumulatorTest, ExcludesWarmupFromPlottedIterationsAndHistogram) {
  using namespace ast::helpers;

  ColorMapRegistry registry;
  const ColorMap* bw = registry.get("BlackWhite");

  // x' = x + 1, y' = 0; min_lifetime = 2, max_lifetime = 5.
  CameraSystem system = {
      TransformSystem{
          ast::Transform(ast::Formula{ast::Input(ast::Input::X) + 1.f, 0.f},
                         1.f),
          {}},
      stdx::nullopt,
      Lifetime(2, 5),
  };
  auto rng = std::make_shared<testing::NiceMock<MockRng>>();
  ON_CALL(*rng, randomFloat(_, _)).WillByDefault(Return(0.0f));
  EXPECT_CALL(*rng, randomInt(3, 5)).WillOnce(Return(4));

  CpuAccumulator accumulator(system, 16, 16, bw, rng);
  // Initial lifetime=4, warmup=2:
  // Ticks 1,2: warmup (not plotted)
  // Ticks 3,4: plotted (x=3, 4 => offscreen, still counted)
  // Tick 5: revives with lifetime=5, warmup=2 -> warmup tick (not plotted)
  EXPECT_EQ(accumulator.accumulate(5), 2u);
  EXPECT_EQ(accumulator.plottedIterations(), 2u);
}

TEST(CpuAccumulatorTest,
     DiscardsNonFiniteStepsAndRevivesUnderInfiniteLifetime) {
  using namespace ast::helpers;

  ColorMapRegistry registry;
  const ColorMap* bgr = registry.get("BGR");

  // Step 1 (from x=0): 1/0 = +Inf (non-finite -> expired, NOT plotted)
  // Step 2 (revived to x=0.5): 1/0.5 = 2.0 (finite -> plotted!)
  // Step 3 (from x=2.0): 1/2.0 = 0.5 (finite -> plotted at x=0.5!)
  CameraSystem system = {
      TransformSystem{
          ast::Transform(ast::Formula{1.f / ast::Input(ast::Input::X), 0.f},
                         0.5f),
          {}},
      stdx::nullopt,
      Lifetime(),  // Infinite lifetime
  };
  // Initial createParticle gets (0, 0, 0), revived particle gets (0.5, 0, 0.5).
  auto rng = std::make_shared<SequenceFloatRng>(
      std::vector<float>{0.f, 0.f, 0.f, 0.5f, 0.f, 0.5f});

  CpuAccumulator accumulator(system, 8, 8, bgr, rng);
  // Tick 1: 1/0 = Inf -> discarded & expired (plotted=0)
  // Tick 2: revives to x=0.5 -> 1/0.5 = 2.0 -> finite (plotted=1, offscreen)
  // Tick 3: from x=2.0 -> 1/2.0 = 0.5 -> finite (plotted=2, onscreen (6, 4))
  EXPECT_EQ(accumulator.accumulate(3), 2u);
  EXPECT_EQ(accumulator.plottedIterations(), 2u);
  EXPECT_FLOAT_EQ(accumulator.histogram()(6, 4)->a, 1.f);
  EXPECT_FLOAT_EQ(accumulator.histogram()(6, 4)->g, 1.f);
}

TEST(CpuAccumulatorTest,
     AppliesCameraWithoutTrajectoryFeedbackAndDiscardsNonFiniteCamera) {
  using namespace ast::helpers;

  ColorMapRegistry registry;
  const ColorMap* bw = registry.get("BlackWhite");

  // System steps x' = x + 1.0, y' = 0; Camera computes x_cam = 0.5 / x.
  // Initial x = -1.0:
  // Tick 1: system x = 0.0 -> camera = 0.5/0 = +Inf (discarded!)
  // Tick 2: system x = 1.0 (trajectory preserved!) -> camera = 0.5/1.0 = 0.5
  // (onscreen at (6, 4), plotted=1!)
  CameraSystem system = {
      TransformSystem{
          ast::Transform(ast::Formula{ast::Input(ast::Input::X) + 1.0f, 0.f},
                         1.f),
          {}},
      TransformSystem{
          ast::Transform(ast::Formula{0.5f / ast::Input(ast::Input::X), 0.f},
                         ast::Input(ast::Input::COLOR)),
          {}},
      Lifetime(),
  };
  auto rng =
      std::make_shared<SequenceFloatRng>(std::vector<float>{-1.0f, 0.0f, 1.0f});

  CpuAccumulator accumulator(system, 8, 8, bw, rng);
  EXPECT_EQ(accumulator.accumulate(2), 1u);
  EXPECT_EQ(accumulator.plottedIterations(), 1u);
  // At x=1.0, camera output is (0.5, 0) -> pixel (6, 4).
  EXPECT_FLOAT_EQ(accumulator.histogram()(6, 4)->a, 1.f);
}

TEST(CpuAccumulatorTest, ResetsAtomicallyOnSystemColorMapSizeAndReset) {
  ColorMapRegistry registry;
  const ColorMap* bgr = registry.get("BGR");
  const ColorMap* bw = registry.get("BlackWhite");

  CameraSystem sys1 = {
      TransformSystem{ast::Transform(ast::Formula{0.f, 0.f}, 0.f), {}},
      stdx::nullopt,
      Lifetime(),
  };
  CameraSystem sys2 = {
      TransformSystem{ast::Transform(ast::Formula{0.5f, 0.5f}, 1.f), {}},
      stdx::nullopt,
      Lifetime(),
  };

  auto rng = std::make_shared<StaticRng>(0.0f);
  CpuAccumulator accumulator(sys1, 8, 8, bgr, rng);
  accumulator.accumulate(10);
  ASSERT_EQ(accumulator.plottedIterations(), 10u);
  ASSERT_FLOAT_EQ(accumulator.histogram()(4, 4)->a, 10.f);

  // Swap system -> clears buffer and plotted count immediately.
  accumulator.setSystem(sys2);
  EXPECT_EQ(accumulator.plottedIterations(), 0u);
  EXPECT_FLOAT_EQ(accumulator.histogram()(4, 4)->a, 0.f);
  EXPECT_EQ(accumulator.accumulate(3), 3u);
  EXPECT_FLOAT_EQ(accumulator.histogram()(4, 4)->a, 0.f);
  EXPECT_FLOAT_EQ(accumulator.histogram()(6, 6)->a, 3.f);

  // Swap color map -> clears buffer and plotted count immediately.
  accumulator.setColorMap(bw);
  EXPECT_EQ(accumulator.plottedIterations(), 0u);
  EXPECT_FLOAT_EQ(accumulator.histogram()(6, 6)->a, 0.f);
  EXPECT_EQ(accumulator.accumulate(2), 2u);
  EXPECT_FLOAT_EQ(accumulator.histogram()(6, 6)->r, 2.f);

  // Swap size -> clears buffer and plotted count immediately.
  accumulator.setSize(16, 16);
  EXPECT_EQ(accumulator.width(), 16u);
  EXPECT_EQ(accumulator.height(), 16u);
  EXPECT_EQ(accumulator.plottedIterations(), 0u);
  EXPECT_EQ(accumulator.accumulate(4), 4u);
  EXPECT_FLOAT_EQ(accumulator.histogram()(12, 12)->a, 4.f);

  // Explicit reset -> clears buffer and plotted count.
  accumulator.reset();
  EXPECT_EQ(accumulator.plottedIterations(), 0u);
  EXPECT_FLOAT_EQ(accumulator.histogram()(12, 12)->a, 0.f);
}

TEST(CpuAccumulatorTest,
     MergesMultipleAccumulatorsAndPreservesSeedDeterminism) {
  using namespace ast::helpers;

  ColorMapRegistry registry;
  const ColorMap* rainbow = registry.get("Rainbow");

  CameraSystem system = {
      TransformSystem{
          ast::Transform(ast::Formula{ast::Input(ast::Input::X) * 0.5f + 0.1f,
                                      ast::Input(ast::Input::Y) * 0.5f - 0.1f},
                         0.3f),
          {}},
      stdx::nullopt,
      Lifetime(1, 5),
  };

  CpuAccumulator a1(system, 16, 16, rainbow,
                    std::make_shared<DeterministicPcgRng>(12345ULL));
  CpuAccumulator a2(system, 16, 16, rainbow,
                    std::make_shared<DeterministicPcgRng>(12345ULL));

  a1.accumulate(200);
  a2.accumulate(200);

  EXPECT_EQ(a1.plottedIterations(), a2.plottedIterations());
  EXPECT_GT(a1.plottedIterations(), 0u);
  ASSERT_EQ(a1.histogram().size(), a2.histogram().size());
  EXPECT_EQ(std::memcmp(a1.histogram().data(), a2.histogram().data(),
                        a1.histogram().size() * sizeof(Color)),
            0);

  const uint64_t expectedMerged =
      a1.plottedIterations() + a2.plottedIterations();
  a1.merge(a2);
  EXPECT_EQ(a1.plottedIterations(), expectedMerged);

  CpuAccumulator wrongSize(system, 8, 8, rainbow,
                           std::make_shared<StaticRng>(0.0f));
  EXPECT_THROW(a1.merge(wrongSize), std::invalid_argument);
}

TEST(CpuAccumulatorTest,
     AccumulatePlottedReachesExactTargetDespiteWarmupAndNonFiniteSteps) {
  using namespace ast::helpers;

  ColorMapRegistry registry;
  const ColorMap* bw = registry.get("BlackWhite");

  // x' = x + 1 (always finite), min_lifetime = 2, max_lifetime = 5.
  CameraSystem warmupSystem = {
      TransformSystem{
          ast::Transform(ast::Formula{ast::Input(ast::Input::X) + 1.f, 0.f},
                         1.f),
          {}},
      stdx::nullopt,
      Lifetime(2, 5),
  };
  auto rng = std::make_shared<StaticRng>(0.0f);
  rng->setInt(4);
  CpuAccumulator warmupAcc(warmupSystem, 8, 8, bw, rng);
  EXPECT_EQ(warmupAcc.accumulatePlotted(10), 10u);
  EXPECT_EQ(warmupAcc.plottedIterations(), 10u);

  // Degenerate system where min_lifetime == max_lifetime (never plots) must
  // terminate cleanly with 0 plotted iterations instead of hanging.
  CameraSystem neverPlots = {
      TransformSystem{ast::Transform(ast::Formula{0.f, 0.f}, 1.f), {}},
      stdx::nullopt,
      Lifetime(3, 3),
  };
  rng->setInt(3);
  CpuAccumulator degenerateAcc(neverPlots, 8, 8, bw, rng);
  EXPECT_EQ(degenerateAcc.accumulatePlotted(100), 0u);
  EXPECT_EQ(degenerateAcc.plottedIterations(), 0u);
}

}  // namespace
}  // namespace chaoskit::core
