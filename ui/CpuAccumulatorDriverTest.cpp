#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <cstdint>
#include <optional>
#include "CpuAccumulatorDriver.h"
#include "ast/Formula.h"
#include "ast/Parameter.h"
#include "ast/Transform.h"
#include "core/CameraSystem.h"
#include "core/Color.h"
#include "core/ColorMap.h"
#include "core/ColorMapRegistry.h"
#include "core/HistogramBuffer.h"
#include "core/Lifetime.h"
#include "core/TransformSystem.h"

namespace chaoskit::ui {
namespace {

class CpuAccumulatorDriverTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    if (QCoreApplication::instance() == nullptr) {
      static int argc = 1;
      static char name[] = "ui_driver_test";
      static char* argv[] = {name, nullptr};
      static QCoreApplication app(argc, argv);
    }
  }
};

core::CameraSystem makeConstantSystem(float x, float y, float color) {
  return core::CameraSystem{
      core::TransformSystem{ast::Transform(ast::Formula{x, y}, color), {}},
      stdx::nullopt,
      core::Lifetime(),
  };
}

TEST_F(CpuAccumulatorDriverTest, AccumulatesBatchesAndTracksPlottedIterations) {
  core::ColorMapRegistry registry;
  const core::ColorMap* bgr = registry.get("BGR");
  ASSERT_NE(bgr, nullptr);

  CpuAccumulatorDriver driver;
  driver.setSize(16, 16);
  driver.setColorMap(bgr);
  driver.setSystem(makeConstantSystem(0.0f, 0.0f, 0.5f));

  QSignalSpy spy(&driver, &CpuAccumulatorDriver::batchCompleted);
  driver.setRunning(true);
  EXPECT_TRUE(driver.isRunning());

  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return spy.size() >= 2 && driver.plottedIterations() > 0; },
      1000));
  driver.setRunning(false);
  EXPECT_FALSE(driver.isRunning());

  core::HistogramBuffer buffer;
  uint64_t plotted = 0;
  driver.snapshot(buffer, &plotted);
  EXPECT_GT(plotted, 0u);
  EXPECT_EQ(buffer.width(), 16u);
  EXPECT_EQ(buffer.height(), 16u);

  const core::Color* hit = buffer(8, 8);
  EXPECT_FLOAT_EQ(hit->r, 0.0f);
  EXPECT_FLOAT_EQ(hit->g, static_cast<float>(plotted));
  EXPECT_FLOAT_EQ(hit->b, 0.0f);
  EXPECT_FLOAT_EQ(hit->a, static_cast<float>(plotted));
}

TEST_F(CpuAccumulatorDriverTest,
       AtomicSwapsPreventStaleHitsAcrossSystemColorMapAndSize) {
  core::ColorMapRegistry registry;
  const core::ColorMap* bgr = registry.get("BGR");
  const core::ColorMap* bw = registry.get("BlackWhite");
  ASSERT_NE(bgr, nullptr);
  ASSERT_NE(bw, nullptr);

  CpuAccumulatorDriver driver;
  driver.setSize(16, 16);
  driver.setColorMap(bgr);
  // Plots solely at (0.5, 0.5) => pixel (12, 12), color 0.0 => BGR (0, 0, 1).
  driver.setSystem(makeConstantSystem(0.5f, 0.5f, 0.0f));

  QSignalSpy spy(&driver, &CpuAccumulatorDriver::batchCompleted);
  driver.setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return spy.size() >= 1 && driver.plottedIterations() > 0; },
      1000));

  // Swap system while actively running to (-0.5, -0.5) => pixel (4, 4).
  for (int i = 0; i < 10; ++i) {
    driver.setSystem(makeConstantSystem(-0.5f, -0.5f, 1.0f));
    // Immediately after swap, snapshot must be cleared with 0 plotted count.
    core::HistogramBuffer cleared;
    uint64_t clearedPlotted = 999;
    driver.snapshot(cleared, &clearedPlotted);
    EXPECT_EQ(clearedPlotted, 0u);
    EXPECT_FLOAT_EQ(cleared(12, 12)->a, 0.0f);
  }

  spy.clear();
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return spy.size() >= 2 && driver.plottedIterations() > 0; },
      1000));

  core::HistogramBuffer afterSystemSwap;
  uint64_t plottedAfterSystem = 0;
  driver.snapshot(afterSystemSwap, &plottedAfterSystem);
  EXPECT_GT(plottedAfterSystem, 0u);
  EXPECT_FLOAT_EQ(afterSystemSwap(12, 12)->a, 0.0f);
  EXPECT_FLOAT_EQ(afterSystemSwap(4, 4)->a,
                  static_cast<float>(plottedAfterSystem));
  EXPECT_FLOAT_EQ(afterSystemSwap(4, 4)->r,
                  static_cast<float>(plottedAfterSystem));

  // Swap only ColorMap while running at (0, 0) with color=0.0:
  // BGR(0.0) = (0, 0, 1) whereas BlackWhite(0.0) = (0, 0, 0).
  driver.setSystem(makeConstantSystem(0.0f, 0.0f, 0.0f));
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return driver.plottedIterations() > 0; }, 1000));
  driver.setColorMap(bw);
  {
    core::HistogramBuffer cleared;
    uint64_t clearedPlotted = 999;
    driver.snapshot(cleared, &clearedPlotted);
    EXPECT_EQ(clearedPlotted, 0u);
    EXPECT_FLOAT_EQ(cleared(8, 8)->a, 0.0f);
  }
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return driver.plottedIterations() > 0; }, 1000));

  core::HistogramBuffer afterColorMapSwap;
  uint64_t plottedAfterColor = 0;
  driver.snapshot(afterColorMapSwap, &plottedAfterColor);
  EXPECT_GT(plottedAfterColor, 0u);
  EXPECT_FLOAT_EQ(afterColorMapSwap(8, 8)->r, 0.0f);
  EXPECT_FLOAT_EQ(afterColorMapSwap(8, 8)->g, 0.0f);
  EXPECT_FLOAT_EQ(afterColorMapSwap(8, 8)->b, 0.0f);
  EXPECT_FLOAT_EQ(afterColorMapSwap(8, 8)->a,
                  static_cast<float>(plottedAfterColor));

  // Swap size while running: 16x16 -> 32x32.
  driver.setSize(32, 32);
  {
    core::HistogramBuffer cleared;
    uint64_t clearedPlotted = 999;
    driver.snapshot(cleared, &clearedPlotted);
    EXPECT_EQ(cleared.width(), 32u);
    EXPECT_EQ(cleared.height(), 32u);
    EXPECT_EQ(clearedPlotted, 0u);
    EXPECT_FLOAT_EQ(cleared(16, 16)->a, 0.0f);
  }
  spy.clear();
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return spy.size() >= 2 && driver.plottedIterations() > 0; },
      1000));
  driver.setRunning(false);

  core::HistogramBuffer afterResize;
  uint64_t plottedAfterResize = 0;
  driver.snapshot(afterResize, &plottedAfterResize);
  EXPECT_EQ(afterResize.width(), 32u);
  EXPECT_EQ(afterResize.height(), 32u);
  EXPECT_GT(plottedAfterResize, 0u);
  EXPECT_FLOAT_EQ(afterResize(16, 16)->a,
                  static_cast<float>(plottedAfterResize));
}

TEST_F(CpuAccumulatorDriverTest, ClearResetsBufferAndPlottedCount) {
  core::ColorMapRegistry registry;
  const core::ColorMap* bgr = registry.get("BGR");
  ASSERT_NE(bgr, nullptr);

  CpuAccumulatorDriver driver;
  driver.setSize(8, 8);
  driver.setColorMap(bgr);
  driver.setSystem(makeConstantSystem(0.0f, 0.0f, 0.5f));
  driver.setRunning(true);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return driver.plottedIterations() > 0; }, 1000));
  driver.setRunning(false);

  driver.clear();
  core::HistogramBuffer buffer;
  uint64_t plotted = 999;
  driver.snapshot(buffer, &plotted);
  EXPECT_EQ(plotted, 0u);
  EXPECT_FLOAT_EQ(buffer(4, 4)->a, 0.0f);
}

TEST_F(CpuAccumulatorDriverTest, StopsAndEmitsErrorOnInvalidSystem) {
  CpuAccumulatorDriver driver;
  driver.setSize(8, 8);
  // Missing Parameter(0) triggers MissingParameterError.
  driver.setSystem(core::CameraSystem{
      core::TransformSystem{
          ast::Transform(ast::Formula{ast::Parameter(0), 0.0f}, 0.5f), {}},
      stdx::nullopt,
      core::Lifetime(),
  });

  QSignalSpy errorSpy(&driver, &CpuAccumulatorDriver::errorOccurred);
  driver.setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return errorSpy.size() >= 1; }, 1000));
  EXPECT_FALSE(driver.isRunning());
}

TEST_F(CpuAccumulatorDriverTest,
       MultiThreadedDriverSumsHistogramsAndPlottedIterationsAcrossWorkers) {
  core::ColorMapRegistry registry;
  const core::ColorMap* bgr = registry.get("BGR");
  ASSERT_NE(bgr, nullptr);

  CpuAccumulatorDriver defaultDriver;
  EXPECT_GE(defaultDriver.threadCount(), 1u);

  CpuAccumulatorDriver driver(nullptr, 4);
  EXPECT_EQ(driver.threadCount(), 4u);

  driver.setSize(16, 16);
  driver.setColorMap(bgr);
  driver.setSystem(makeConstantSystem(0.0f, 0.0f, 0.5f));

  QSignalSpy spy(&driver, &CpuAccumulatorDriver::batchCompleted);
  driver.setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return spy.size() >= 3 && driver.plottedIterations() >= 8192u; },
      2000));

  driver.clear();
  {
    core::HistogramBuffer cleared;
    uint64_t clearedPlotted = 999;
    driver.snapshot(cleared, &clearedPlotted);
    EXPECT_EQ(clearedPlotted, 0u);
    EXPECT_FLOAT_EQ(cleared(8, 8)->a, 0.0f);
  }

  spy.clear();
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return spy.size() >= 2 && driver.plottedIterations() >= 4096u; },
      2000));
  driver.setRunning(false);

  core::HistogramBuffer buffer;
  uint64_t plotted = 0;
  driver.snapshot(buffer, &plotted);
  EXPECT_GE(plotted, 4096u);

  const core::Color* hit = buffer(8, 8);
  EXPECT_FLOAT_EQ(hit->r, 0.0f);
  EXPECT_FLOAT_EQ(hit->g, static_cast<float>(plotted));
  EXPECT_FLOAT_EQ(hit->b, 0.0f);
  EXPECT_FLOAT_EQ(hit->a, static_cast<float>(plotted));

  // Verify ~CpuAccumulatorDriver joins all workers cleanly while running.
  driver.setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return driver.plottedIterations() > plotted; }, 2000));
}

}  // namespace
}  // namespace chaoskit::ui
