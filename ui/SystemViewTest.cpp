#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <QColor>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QSizeF>
#include <QTest>
#include <QtGlobal>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include "SystemView.h"
#include "chaoskit.pb.h"
#include "models/ColorMapRegistry.h"
#include "models/ModelFactory.h"
#include "models/ProjectModel.h"

namespace chaoskit::ui {
namespace {

std::unique_ptr<Project> createTestProjectProto(uint32_t width = 32,
                                                uint32_t height = 32) {
  auto project = std::make_unique<Project>();
  project->set_gamma(2.2f);
  project->set_exposure(0.f);
  project->set_vibrancy(0.f);
  project->set_width(width);
  project->set_height(height);
  project->mutable_color_map()->set_name("BGR");

  auto* system = project->mutable_system();
  system->set_min_lifetime(0);
  system->set_max_lifetime(20);

  auto* blend = system->add_blends();
  blend->set_enabled(true);
  blend->set_weight(1.f);
  blend->mutable_coloring_method()->set_single_color(0.5f);

  // Constant-origin DeJong / simple contractive Linear blend so particles
  // stay finite and plot reliably near (0, 0).
  auto* formula = blend->add_formulas();
  formula->set_type("Linear");
  formula->mutable_weight()->set_x(1.f);

  auto* pre = blend->mutable_pre();
  pre->set_m11(0.5f);
  pre->set_m22(0.5f);
  pre->set_m31(0.f);
  pre->set_m32(0.f);

  return project;
}

class SystemViewTest : public ::testing::Test {
 protected:
  void SetUp() override {
    colorMapRegistry_ = std::make_unique<ColorMapRegistry>();
    projectModel_ = modelFactory_.createProjectModel(nullptr);
    projectModel_->colorMap()->setColorMapRegistry(colorMapRegistry_.get());
    projectModel_->setProto(createTestProjectProto().release());

    window_ = std::make_unique<QQuickWindow>();
    window_->resize(64, 64);

    view_ = new SystemView(window_->contentItem());
    view_->setModel(projectModel_);
    view_->setSize(QSizeF(static_cast<double>(projectModel_->width()),
                          static_cast<double>(projectModel_->height())));

    window_->show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(window_.get()));
  }

  void TearDown() override {
    if (view_ != nullptr) {
      view_->setRunning(false);
      view_->setModel(nullptr);
    }
    if (window_) {
      window_->close();
      window_.reset();
    }
    QCoreApplication::processEvents();
    view_ = nullptr;
    delete projectModel_;
    projectModel_ = nullptr;
    colorMapRegistry_.reset();
  }

  std::unique_ptr<ColorMapRegistry> colorMapRegistry_;
  ModelFactory modelFactory_;
  ProjectModel* projectModel_ = nullptr;
  std::unique_ptr<QQuickWindow> window_;
  SystemView* view_ = nullptr;
};

TEST_F(SystemViewTest, ExposesConvergenceDefaultsAndRejectsInvalidTargets) {
  EXPECT_DOUBLE_EQ(view_->targetIterationsPerPixel(), 1000.0);
  EXPECT_FALSE(view_->infiniteAccumulation());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_FALSE(view_->converged());
  EXPECT_FALSE(view_->running());

  QSignalSpy targetSpy(view_, &SystemView::targetIterationsPerPixelChanged);
  view_->setTargetIterationsPerPixel(0.0);
  view_->setTargetIterationsPerPixel(-10.0);
  view_->setTargetIterationsPerPixel(std::numeric_limits<double>::quiet_NaN());
  view_->setTargetIterationsPerPixel(std::numeric_limits<double>::infinity());

  EXPECT_DOUBLE_EQ(view_->targetIterationsPerPixel(), 1000.0);
  EXPECT_EQ(targetSpy.size(), 0);
}

TEST_F(SystemViewTest,
       ConvergesAtTargetIppAndStopsAccumulatingAfterPresentingFrame) {
  view_->setTargetIterationsPerPixel(5.0);
  EXPECT_DOUBLE_EQ(view_->targetIterationsPerPixel(), 5.0);

  QSignalSpy convergedSpy(view_, &SystemView::convergedChanged);
  QSignalSpy ippSpy(view_, &SystemView::currentIterationsPerPixelChanged);
  QSignalSpy runningSpy(view_, &SystemView::runningChanged);
  QSignalSpy frameSpy(window_.get(), &QQuickWindow::frameSwapped);
  const int framesBeforeRun = frameSpy.size();

  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return view_->converged() && frameSpy.size() > framesBeforeRun; },
      5000));

  EXPECT_TRUE(view_->converged());
  EXPECT_TRUE(view_->running());
  EXPECT_EQ(runningSpy.size(), 1);
  EXPECT_EQ(convergedSpy.size(), 1);
  EXPECT_GE(ippSpy.size(), 1);
  EXPECT_GE(view_->currentIterationsPerPixel(), 5.0);
  EXPECT_FALSE(view_->driver()->isRunning());

  // Verify the tone-mapped frame has been presented with non-black content.
  const QImage grabbed = window_->grabWindow();
  ASSERT_FALSE(grabbed.isNull());
  bool hasNonBlackPixel = false;
  for (int y = 0; y < grabbed.height() && !hasNonBlackPixel; ++y) {
    for (int x = 0; x < grabbed.width(); ++x) {
      const QColor px = grabbed.pixelColor(x, y);
      if (px.red() > 0 || px.green() > 0 || px.blue() > 0) {
        hasNonBlackPixel = true;
        break;
      }
    }
  }
  EXPECT_TRUE(hasNonBlackPixel);

  const double convergedIpp = view_->currentIterationsPerPixel();
  const quint64 convergedPlotted = view_->plottedIterations();
  QTest::qWait(200);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), convergedIpp);
  EXPECT_EQ(view_->plottedIterations(), convergedPlotted);
  EXPECT_TRUE(view_->converged());
  EXPECT_TRUE(view_->running());
  EXPECT_FALSE(view_->driver()->isRunning());
}

TEST_F(SystemViewTest, VisualEditsResetAndResumeWhenRunning) {
  view_->setTargetIterationsPerPixel(5.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  // Tone-mapping adjustments while converged must not reset accumulation.
  const double ippBeforeToneMap = view_->currentIterationsPerPixel();
  projectModel_->setGamma(1.8f);
  projectModel_->setExposure(0.5f);
  projectModel_->setVibrancy(0.4f);
  EXPECT_TRUE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippBeforeToneMap);

  // 1. SystemModel::visuallyChanged
  projectModel_->system()->setMinLifetime(1);
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_TRUE(view_->running());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  // 2. ColorMapModel::nameChanged
  projectModel_->colorMap()->setName("Rainbow");
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_TRUE(view_->running());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  // 3. ProjectModel::widthChanged
  projectModel_->setWidth(40);
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_TRUE(view_->running());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  // 4. ProjectModel::heightChanged
  projectModel_->setHeight(40);
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_TRUE(view_->running());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  // 5. SystemView::clear()
  view_->clear();
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_TRUE(view_->running());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  // 6. ProjectModel::reset() (projectChanged)
  projectModel_->reset();
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_TRUE(view_->running());
}

TEST_F(SystemViewTest, VisualEditsResetWithoutResumingWhenManuallyPaused) {
  view_->setTargetIterationsPerPixel(5.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  view_->setRunning(false);
  ASSERT_FALSE(view_->running());
  ASSERT_TRUE(view_->converged());

  // System edit while paused resets state without starting accumulation.
  projectModel_->system()->setMinLifetime(2);
  EXPECT_FALSE(view_->running());
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  QTest::qWait(150);
  EXPECT_FALSE(view_->running());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_EQ(view_->plottedIterations(), 0u);
  EXPECT_FALSE(view_->driver()->isRunning());

  // Accumulate partially, pause again, then edit color map / width / clear.
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return view_->currentIterationsPerPixel() > 0.0; }, 5000));
  view_->setRunning(false);
  ASSERT_FALSE(view_->running());

  projectModel_->colorMap()->setName("Rainbow");
  EXPECT_FALSE(view_->running());
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  QTest::qWait(150);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_FALSE(view_->driver()->isRunning());

  projectModel_->setWidth(24);
  EXPECT_FALSE(view_->running());
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  QTest::qWait(150);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_FALSE(view_->driver()->isRunning());

  view_->clear();
  EXPECT_FALSE(view_->running());
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  QTest::qWait(150);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  EXPECT_FALSE(view_->driver()->isRunning());
}

TEST_F(SystemViewTest,
       RaisingTargetOrEnablingInfiniteResumesWithoutClearingHistogram) {
  view_->setTargetIterationsPerPixel(5.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  const double ippAtFirst = view_->currentIterationsPerPixel();
  const quint64 plottedAtFirst = view_->plottedIterations();
  ASSERT_GE(ippAtFirst, 5.0);

  double minObservedIpp = ippAtFirst;
  auto conn = QObject::connect(
      view_, &SystemView::currentIterationsPerPixelChanged, [&]() {
        minObservedIpp =
            (std::min)(minObservedIpp, view_->currentIterationsPerPixel());
      });

  // Raising target above current IPP clears converged and resumes losslessly.
  view_->setTargetIterationsPerPixel(25.0);
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippAtFirst);
  EXPECT_EQ(view_->plottedIterations(), plottedAtFirst);
  EXPECT_TRUE(view_->running());

  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_GE(view_->currentIterationsPerPixel(), 25.0);
  EXPECT_GT(view_->plottedIterations(), plottedAtFirst);
  EXPECT_GE(minObservedIpp, ippAtFirst);

  // Enabling infinite accumulation resumes without clearing or converging.
  const double ippBeforeInfinite = view_->currentIterationsPerPixel();
  view_->setInfiniteAccumulation(true);
  EXPECT_TRUE(view_->infiniteAccumulation());
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippBeforeInfinite);

  ASSERT_TRUE(QTest::qWaitFor(
      [&]() {
        return view_->currentIterationsPerPixel() > ippBeforeInfinite + 5.0;
      },
      5000));
  EXPECT_FALSE(view_->converged());

  // Disabling infinite accumulation while already above target converges
  // immediately without clearing.
  const double ippBeforeFinite = view_->currentIterationsPerPixel();
  view_->setInfiniteAccumulation(false);
  EXPECT_FALSE(view_->infiniteAccumulation());
  EXPECT_TRUE(view_->converged());
  EXPECT_TRUE(view_->running());
  EXPECT_FALSE(view_->driver()->isRunning());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippBeforeFinite);

  // Raising target very high resumes, then lowering below current IPP
  // immediately converges again without clearing.
  view_->setTargetIterationsPerPixel(100000.0);
  EXPECT_FALSE(view_->converged());
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() {
        return view_->currentIterationsPerPixel() > ippBeforeFinite + 5.0;
      },
      5000));
  EXPECT_FALSE(view_->converged());

  const double ippBeforeLower = view_->currentIterationsPerPixel();
  view_->setTargetIterationsPerPixel(10.0);
  EXPECT_TRUE(view_->converged());
  EXPECT_FALSE(view_->driver()->isRunning());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippBeforeLower);
  EXPECT_GE(minObservedIpp, ippAtFirst);

  QObject::disconnect(conn);
}

}  // namespace
}  // namespace chaoskit::ui

int main(int argc, char** argv) {
  QGuiApplication app(argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
