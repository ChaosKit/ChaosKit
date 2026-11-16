#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <QColor>
#include <QCoreApplication>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QSizeF>
#include <QTemporaryDir>
#include <QTest>
#include <QVariant>
#include <QtGlobal>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include "SystemView.h"
#include "chaoskit.pb.h"
#include "models/BlendModel.h"
#include "models/ColorMapRegistry.h"
#include "models/ColoringMethodModel.h"
#include "models/FormulaModel.h"
#include "models/ModelFactory.h"
#include "models/ProjectModel.h"
#include "models/SystemModel.h"
#include "models/TransformModel.h"

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
       UsesGpuComputeWhenSupportedAndClearsHistogramWhilePaused) {
  QTemporaryDir tempDir;
  ASSERT_TRUE(tempDir.isValid());

#if defined(Q_OS_MACOS)
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return view_->gpuComputeActive(); }, 5000));
#endif

  view_->setTargetIterationsPerPixel(5.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  if (view_->gpuComputeActive()) {
    EXPECT_EQ(view_->compiledPipelineCount(), 1u);
    EXPECT_FALSE(view_->driver()->isRunning());
    EXPECT_EQ(view_->driver()->plottedIterations(), 0u);
  } else {
    EXPECT_EQ(view_->compiledPipelineCount(), 0u);
  }

  // Pause and clear: the exported frame must become completely black without
  // advancing accumulation.
  view_->setRunning(false);
  view_->clear();
  EXPECT_EQ(view_->plottedIterations(), 0u);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);

  QSignalSpy succeededSpy(view_, &SystemView::imageExportSucceeded);
  QSignalSpy failedSpy(view_, &SystemView::imageExportFailed);
  const QString clearedPath = tempDir.filePath("cleared_paused.png");
  view_->exportImage(clearedPath);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 1; }, 5000));
  EXPECT_EQ(failedSpy.size(), 0);

  const QImage clearedImg(clearedPath);
  ASSERT_FALSE(clearedImg.isNull());
  EXPECT_EQ(clearedImg.size(), QSize(32, 32));
  for (int y = 0; y < clearedImg.height(); ++y) {
    for (int x = 0; x < clearedImg.width(); ++x) {
      const QColor px = clearedImg.pixelColor(x, y);
      EXPECT_EQ(px.red(), 0);
      EXPECT_EQ(px.green(), 0);
      EXPECT_EQ(px.blue(), 0);
    }
  }
  EXPECT_EQ(view_->plottedIterations(), 0u);
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

TEST_F(SystemViewTest, ExportsExactProjectDimensionsAndRejectsInvalidPaths) {
  QSignalSpy succeededSpy(view_, &SystemView::imageExportSucceeded);
  QSignalSpy failedSpy(view_, &SystemView::imageExportFailed);

  // Empty path fails cleanly.
  view_->exportImage(QString());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return failedSpy.size() == 1; }, 5000));
  EXPECT_EQ(succeededSpy.size(), 0);
  EXPECT_FALSE(failedSpy.at(0).at(1).toString().isEmpty());

  // Unwritable / non-existent directory fails cleanly.
  QTemporaryDir tempDir;
  ASSERT_TRUE(tempDir.isValid());
  const QString badPath = tempDir.filePath("missing_subdir/export.png");
  view_->exportImage(badPath);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return failedSpy.size() == 2; }, 5000));
  EXPECT_EQ(succeededSpy.size(), 0);
  EXPECT_EQ(failedSpy.at(1).at(0).toString(), badPath);
  EXPECT_FALSE(failedSpy.at(1).at(1).toString().isEmpty());
  EXPECT_FALSE(QFileInfo::exists(badPath));

  // Missing project model fails cleanly.
  view_->setModel(nullptr);
  const QString noModelPath = tempDir.filePath("no_model.png");
  view_->exportImage(noModelPath);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return failedSpy.size() == 3; }, 5000));
  EXPECT_EQ(succeededSpy.size(), 0);
  EXPECT_EQ(failedSpy.at(2).at(0).toString(), noModelPath);
  EXPECT_FALSE(failedSpy.at(2).at(1).toString().isEmpty());
  view_->setModel(projectModel_);

  // Accumulate some iterations and export to a valid PNG file.
  view_->setTargetIterationsPerPixel(5.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  const QString goodPath = tempDir.filePath("render.png");
  view_->exportImage(goodPath);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 1; }, 5000));
  EXPECT_EQ(failedSpy.size(), 3);
  EXPECT_EQ(succeededSpy.at(0).at(0).toString(), goodPath);

  QImage exported(goodPath);
  ASSERT_FALSE(exported.isNull());
  EXPECT_EQ(exported.width(), static_cast<int>(projectModel_->width()));
  EXPECT_EQ(exported.height(), static_cast<int>(projectModel_->height()));

  bool hasNonBlackPixel = false;
  for (int y = 0; y < exported.height() && !hasNonBlackPixel; ++y) {
    for (int x = 0; x < exported.width(); ++x) {
      const QColor px = exported.pixelColor(x, y);
      if (px.red() > 0 || px.green() > 0 || px.blue() > 0) {
        hasNonBlackPixel = true;
        break;
      }
    }
  }
  EXPECT_TRUE(hasNonBlackPixel);
}

TEST_F(SystemViewTest,
       ExportsWhilePausedOrConvergedWithoutAdvancingOrClearing) {
  QTemporaryDir tempDir;
  ASSERT_TRUE(tempDir.isValid());

  QSignalSpy succeededSpy(view_, &SystemView::imageExportSucceeded);
  QSignalSpy failedSpy(view_, &SystemView::imageExportFailed);

  // 1. Export while paused mid-accumulation (below target IPP).
  view_->setTargetIterationsPerPixel(500.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return view_->currentIterationsPerPixel() >= 1.0; }, 5000));

  view_->setRunning(false);
  ASSERT_FALSE(view_->running());
  ASSERT_FALSE(view_->converged());
  QTest::qWait(50);

  const double pausedIpp = view_->currentIterationsPerPixel();
  const quint64 pausedPlotted = view_->plottedIterations();
  ASSERT_GT(pausedPlotted, 0u);
  ASSERT_FALSE(view_->driver()->isRunning());

  const QString pausedPath1 = tempDir.filePath("paused_1.png");
  view_->exportImage(pausedPath1);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 1; }, 5000));
  EXPECT_EQ(failedSpy.size(), 0);

  QTest::qWait(150);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), pausedIpp);
  EXPECT_EQ(view_->plottedIterations(), pausedPlotted);
  EXPECT_FALSE(view_->running());
  EXPECT_FALSE(view_->converged());
  EXPECT_FALSE(view_->driver()->isRunning());

  const QString pausedPath2 = tempDir.filePath("paused_2.png");
  view_->exportImage(pausedPath2);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 2; }, 5000));
  EXPECT_EQ(failedSpy.size(), 0);

  const QImage pausedImg1(pausedPath1);
  const QImage pausedImg2(pausedPath2);
  ASSERT_FALSE(pausedImg1.isNull());
  ASSERT_FALSE(pausedImg2.isNull());
  EXPECT_EQ(pausedImg1.size(), QSize(32, 32));
  EXPECT_EQ(pausedImg1, pausedImg2);

  // Tone-mapping edits while paused take effect on export without altering IPP.
  projectModel_->setExposure(1.2f);
  const QString pausedBrightPath = tempDir.filePath("paused_bright.png");
  view_->exportImage(pausedBrightPath);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 3; }, 5000));
  EXPECT_EQ(view_->plottedIterations(), pausedPlotted);
  const QImage pausedBrightImg(pausedBrightPath);
  ASSERT_FALSE(pausedBrightImg.isNull());
  EXPECT_NE(pausedBrightImg, pausedImg1);

  // 2. Export while converged (running == true, converged == true).
  projectModel_->setExposure(0.0f);
  view_->setTargetIterationsPerPixel(5.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  QTest::qWait(50);

  const double convergedIpp = view_->currentIterationsPerPixel();
  const quint64 convergedPlotted = view_->plottedIterations();
  ASSERT_FALSE(view_->driver()->isRunning());

  const QString convPath1 = tempDir.filePath("converged_1.png");
  view_->exportImage(convPath1);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 4; }, 5000));

  QTest::qWait(150);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), convergedIpp);
  EXPECT_EQ(view_->plottedIterations(), convergedPlotted);
  EXPECT_TRUE(view_->converged());
  EXPECT_FALSE(view_->driver()->isRunning());

  const QString convPath2 = tempDir.filePath("converged_2.png");
  view_->exportImage(convPath2);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 5; }, 5000));
  EXPECT_EQ(QImage(convPath1), QImage(convPath2));

  // 3. Export while actively accumulating.
  view_->setInfiniteAccumulation(true);
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return view_->currentIterationsPerPixel() > convergedIpp + 2.0; },
      5000));
  const QString activePath = tempDir.filePath("active.png");
  view_->exportImage(activePath);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 6; }, 5000));
  EXPECT_EQ(failedSpy.size(), 0);
  const QImage activeImg(activePath);
  ASSERT_FALSE(activeImg.isNull());
  EXPECT_EQ(activeImg.size(), QSize(32, 32));
}

TEST_F(SystemViewTest,
       ExportDimensionsIgnoreZoomTranslationScaleAndWindowSize) {
  QTemporaryDir tempDir;
  ASSERT_TRUE(tempDir.isValid());

  constexpr uint32_t kProjectWidth = 120;
  constexpr uint32_t kProjectHeight = 80;
  projectModel_->setWidth(kProjectWidth);
  projectModel_->setHeight(kProjectHeight);

  // Host SystemView inside a scaled QML container with non-unit zoom,
  // non-zero translation, and non-unit scale inside a smaller window.
  qmlRegisterType<SystemView>("ChaosKitTest", 1, 0, "TestSystemView");

  QQmlEngine engine;
  QQmlComponent component(&engine);
  component.setData(
      R"QML(
      import QtQuick
      import ChaosKitTest

      Item {
        id: container
        property real zoom: 3.5
        property real tx: 180.0
        property real ty: -120.0
        property real itemScale: 0.5
        property alias systemView: view

        width: 50
        height: 40
        scale: 2.25

        TestSystemView {
          id: view
          width: 120
          height: 80
          scale: container.itemScale
          transform: [
            Scale {
              origin.x: view.width * 0.5
              origin.y: view.height * 0.5
              xScale: container.zoom
              yScale: container.zoom
            },
            Translate {
              x: container.tx
              y: container.ty
            }
          ]
        }
      }
      )QML",
      QUrl());
  ASSERT_TRUE(component.isReady()) << qPrintable(component.errorString());

  std::unique_ptr<QObject> rootObj(component.create());
  auto* container = qobject_cast<QQuickItem*>(rootObj.get());
  ASSERT_NE(container, nullptr);
  auto* qmlView = container->property("systemView").value<SystemView*>();
  ASSERT_NE(qmlView, nullptr);

  // Resize host window smaller than the project dimensions.
  window_->resize(50, 40);
  container->setParentItem(window_->contentItem());
  qmlView->setModel(projectModel_);
  qmlView->setTargetIterationsPerPixel(5.0);
  qmlView->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return qmlView->converged(); }, 5000));
  // Allow any already-started CPU worker batch to settle before snapshotting.
  QTest::qWait(50);
  const quint64 settledDriverPlotted = qmlView->driver()->plottedIterations();

  QSignalSpy succeededSpy(qmlView, &SystemView::imageExportSucceeded);
  QSignalSpy failedSpy(qmlView, &SystemView::imageExportFailed);

  const QString transformedPath = tempDir.filePath("transformed.png");
  qmlView->exportImage(transformedPath);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 1; }, 5000));
  EXPECT_EQ(failedSpy.size(), 0);
  EXPECT_EQ(qmlView->driver()->plottedIterations(), settledDriverPlotted);

  const QImage transformedImg(transformedPath);
  ASSERT_FALSE(transformedImg.isNull());
  EXPECT_EQ(transformedImg.width(), static_cast<int>(kProjectWidth));
  EXPECT_EQ(transformedImg.height(), static_cast<int>(kProjectHeight));

  // Verify the contractive fractal stays centered in the exported image rather
  // than shifted or clipped by viewport pan/zoom.
  const QColor centerPx =
      transformedImg.pixelColor(kProjectWidth / 2, kProjectHeight / 2);
  EXPECT_GT(centerPx.red() + centerPx.green() + centerPx.blue(), 0);

  // Reset viewport transforms/window size and confirm identical pixel output.
  container->setScale(1.0);
  container->setProperty("itemScale", 1.0);
  container->setProperty("zoom", 1.0);
  container->setProperty("tx", 0.0);
  container->setProperty("ty", 0.0);
  window_->resize(160, 120);
  QCoreApplication::processEvents();

  const QString baselinePath = tempDir.filePath("baseline.png");
  qmlView->exportImage(baselinePath);
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return succeededSpy.size() == 2; }, 5000));
  EXPECT_EQ(failedSpy.size(), 0);

  const QImage baselineImg(baselinePath);
  ASSERT_FALSE(baselineImg.isNull());
  EXPECT_EQ(transformedImg, baselineImg);

  qmlView->setRunning(false);
  qmlView->setModel(nullptr);
}

QImage exportProjectImage(SystemView* view, const QString& path) {
  QSignalSpy succeededSpy(view, &SystemView::imageExportSucceeded);
  QSignalSpy failedSpy(view, &SystemView::imageExportFailed);
  view->exportImage(path);
  if (!QTest::qWaitFor([&]() { return succeededSpy.size() == 1; }, 5000) ||
      failedSpy.size() != 0) {
    return QImage();
  }
  return QImage(path);
}

std::unique_ptr<Project> createMultiFeatureProjectProto() {
  auto project = std::make_unique<Project>();
  project->set_gamma(2.2f);
  project->set_exposure(0.f);
  project->set_vibrancy(0.f);
  project->set_width(32);
  project->set_height(32);
  project->mutable_color_map()->set_name("BGR");

  auto* system = project->mutable_system();
  system->set_min_lifetime(0);
  system->set_max_lifetime(20);

  // Blend 0: contractive affine + DeJong + Linear formulas + SingleColor.
  auto* blend0 = system->add_blends();
  blend0->set_enabled(true);
  blend0->set_weight(1.f);
  blend0->mutable_coloring_method()->set_single_color(0.15f);
  auto* pre0 = blend0->mutable_pre();
  pre0->set_m11(0.25f);
  pre0->set_m22(0.25f);
  pre0->set_m31(0.f);
  pre0->set_m32(0.f);

  auto* dejong = blend0->add_formulas();
  dejong->set_type("DeJong");
  dejong->mutable_weight()->set_x(1.f);
  dejong->mutable_weight()->set_y(1.f);
  dejong->add_params(0.9379667f);
  dejong->add_params(1.9387093f);
  dejong->add_params(-0.1580897f);
  dejong->add_params(-1.4300701f);

  auto* linear0 = blend0->add_formulas();
  linear0->set_type("Linear");
  linear0->mutable_weight()->set_x(0.05f);
  linear0->mutable_weight()->set_y(0.05f);

  // Blend 1: tiny weight initially, located near (-0.5, -0.5).
  auto* blend1 = system->add_blends();
  blend1->set_enabled(true);
  blend1->set_weight(0.01f);
  blend1->mutable_coloring_method()->set_single_color(0.15f);
  auto* pre1 = blend1->mutable_pre();
  pre1->set_m11(0.25f);
  pre1->set_m22(0.25f);
  pre1->set_m31(-0.5f);
  pre1->set_m32(-0.5f);

  auto* linear1 = blend1->add_formulas();
  linear1->set_type("Linear");
  linear1->mutable_weight()->set_x(1.f);
  linear1->mutable_weight()->set_y(1.f);

  return project;
}

TEST_F(SystemViewTest,
       ContinuousEditsAndColorMapSwitchUpdateOutputWithoutRecompiling) {
  QTemporaryDir tempDir;
  ASSERT_TRUE(tempDir.isValid());

  projectModel_->setProto(createMultiFeatureProjectProto().release());
  view_->setTargetIterationsPerPixel(25.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  const quint64 baselineCompileCount = view_->compiledPipelineCount();
  if (view_->gpuComputeActive()) {
    ASSERT_GE(baselineCompileCount, 1u);
  }

  QImage prevImg =
      exportProjectImage(view_, tempDir.filePath("00_baseline.png"));
  ASSERT_FALSE(prevImg.isNull());

  auto* system = projectModel_->system();
  auto* blend0 = system->blends()->at(0);
  auto* blend1 = system->blends()->at(1);
  auto* dejong = blend0->formulas()->at(0);
  auto* linear0 = blend0->formulas()->at(1);

  auto verifyContinuousMutation = [&](const char* tag, auto&& mutator,
                                      bool expectPixelDiff) {
    SCOPED_TRACE(tag);
    mutator();
    EXPECT_FALSE(view_->converged());
    EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
    ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
    EXPECT_EQ(view_->compiledPipelineCount(), baselineCompileCount);

    const QImage nextImg =
        exportProjectImage(view_, tempDir.filePath(QString("%1.png").arg(tag)));
    ASSERT_FALSE(nextImg.isNull());
    if (expectPixelDiff) {
      EXPECT_NE(nextImg, prevImg);
    }
    prevImg = nextImg;
  };

  // 1. Affine Transform mutation
  verifyContinuousMutation(
      "01_affine", [&]() { blend0->pre()->setTranslationX(0.35f); }, true);

  // 2. Formula parameter mutation
  verifyContinuousMutation(
      "02_formula_param", [&]() { dejong->setParam(0, -1.8f); }, true);

  // 3. Blend weight mutation
  verifyContinuousMutation(
      "03_blend_weight", [&]() { blend1->setWeight(25.0f); }, true);

  // Restore blend0 dominance before testing blend0's formula & coloring.
  blend1->setWeight(0.01f);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  prevImg = exportProjectImage(view_, tempDir.filePath("03b_restored.png"));
  ASSERT_FALSE(prevImg.isNull());
  EXPECT_EQ(view_->compiledPipelineCount(), baselineCompileCount);

  // 4. Formula weight mutation (stays contractive: 0.25 * 2.0 = 0.5 < 1.0)
  verifyContinuousMutation(
      "04_formula_weight", [&]() { linear0->setWeight(2.0f); }, true);

  // 5. Coloring Method parameter mutation (0.15 -> 0.5 on BGR)
  verifyContinuousMutation(
      "05_coloring_param",
      [&]() { blend0->coloringMethod()->setParameter(0.5f); }, true);

  // 6. Lifetime bounds mutations (finite bounds + infinite toggle)
  verifyContinuousMutation(
      "06a_lifetime_max", [&]() { system->setMaxLifetime(1); }, true);
  verifyContinuousMutation(
      "06b_lifetime_max_restore", [&]() { system->setMaxLifetime(20); }, true);
  verifyContinuousMutation(
      "06c_lifetime_min", [&]() { system->setMinLifetime(12); }, true);

  system->setInfiniteLifetime(true);
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), baselineCompileCount);

  system->setInfiniteLifetime(false);
  EXPECT_FALSE(view_->converged());
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), baselineCompileCount);

  // 7. Color Map switch (LUT upload only, zero recompilation)
  verifyContinuousMutation(
      "07_colormap", [&]() { projectModel_->colorMap()->setName("Rainbow"); },
      true);
}

TEST_F(SystemViewTest,
       ToneMappingEditsUpdateOutputWithoutClearingOrRecompiling) {
  QTemporaryDir tempDir;
  ASSERT_TRUE(tempDir.isValid());

  // Use a non-achromatic color coordinate so vibrancy alters RGB saturation.
  projectModel_->system()->blends()->at(0)->coloringMethod()->setParameter(
      0.2f);
  view_->setTargetIterationsPerPixel(25.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  const quint64 plottedBefore = view_->plottedIterations();
  const double ippBefore = view_->currentIterationsPerPixel();
  const quint64 compilesBefore = view_->compiledPipelineCount();
  ASSERT_GT(plottedBefore, 0u);

  const QImage baseImg =
      exportProjectImage(view_, tempDir.filePath("base.png"));
  ASSERT_FALSE(baseImg.isNull());

  projectModel_->setGamma(1.1f);
  EXPECT_TRUE(view_->converged());
  EXPECT_EQ(view_->plottedIterations(), plottedBefore);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippBefore);
  EXPECT_EQ(view_->compiledPipelineCount(), compilesBefore);
  const QImage gammaImg =
      exportProjectImage(view_, tempDir.filePath("gamma.png"));
  ASSERT_FALSE(gammaImg.isNull());
  EXPECT_NE(gammaImg, baseImg);

  projectModel_->setExposure(1.8f);
  EXPECT_TRUE(view_->converged());
  EXPECT_EQ(view_->plottedIterations(), plottedBefore);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippBefore);
  EXPECT_EQ(view_->compiledPipelineCount(), compilesBefore);
  const QImage exposureImg =
      exportProjectImage(view_, tempDir.filePath("exposure.png"));
  ASSERT_FALSE(exposureImg.isNull());
  EXPECT_NE(exposureImg, gammaImg);

  projectModel_->setVibrancy(1.0f);
  EXPECT_TRUE(view_->converged());
  EXPECT_EQ(view_->plottedIterations(), plottedBefore);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), ippBefore);
  EXPECT_EQ(view_->compiledPipelineCount(), compilesBefore);
  const QImage vibrancyImg =
      exportProjectImage(view_, tempDir.filePath("vibrancy.png"));
  ASSERT_FALSE(vibrancyImg.isNull());
  EXPECT_NE(vibrancyImg, exposureImg);
}

TEST_F(SystemViewTest,
       StructuralSystemEditsCompileUnseenAndReuseCachedPipelines) {
#if defined(Q_OS_MACOS)
  ASSERT_TRUE(
      QTest::qWaitFor([&]() { return view_->gpuComputeActive(); }, 5000));
#endif
  if (!view_->gpuComputeActive()) {
    GTEST_SKIP() << "Pipeline compilation test requires QRhi::Compute support";
  }

  view_->setTargetIterationsPerPixel(5.0);
  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  quint64 expectedCompiles = view_->compiledPipelineCount();
  ASSERT_EQ(expectedCompiles, 1u);

  auto* system = projectModel_->system();

  // 1. Add blend -> compiles unseen pipeline; delete -> reuses initial cache.
  system->addBlend();
  EXPECT_FALSE(view_->converged());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  system->deleteBlendAt(1);
  EXPECT_FALSE(view_->converged());
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  // 2. Add formula -> compiles unseen; delete formula -> reuses cached.
  auto* blend0 = system->blends()->at(0);
  blend0->addFormula("Sinusoidal");
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  blend0->deleteFormulaAt(1);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  // 3. Change formula type -> compiles unseen; revert -> reuses cached.
  auto* formula0 = blend0->formulas()->at(0);
  formula0->setType("Spherical");
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  formula0->setType("Linear");
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  // 4. Change Coloring Method type -> compiles unseen; revert -> reuses cached.
  blend0->coloringMethod()->setTypeIndex(2);  // Distance
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  blend0->coloringMethod()->setTypeIndex(1);  // SingleColor
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  // 5. Toggle Camera on -> compiles unseen; continuous Camera transform edit
  // does not recompile; toggle Camera off -> reuses cached.
  system->cameraBlend()->setEnabled(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  system->cameraBlend()->addPre();
  // Adding a pre-transform to Camera changes AST structure once.
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  system->cameraBlend()->pre()->setTranslationX(0.25f);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  system->cameraBlend()->removePre();
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));

  system->cameraBlend()->setEnabled(false);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  // 6. Toggle Isolated Blend on -> compiles unseen; clear -> reuses cached.
  {
    auto isolatedProto = std::make_unique<Project>(*projectModel_->proto());
    isolatedProto->mutable_system()->mutable_isolated_blend()->set_index(0);
    projectModel_->setProto(isolatedProto.release());
  }
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  {
    auto unisolatedProto = std::make_unique<Project>(*projectModel_->proto());
    unisolatedProto->mutable_system()->clear_isolated_blend();
    projectModel_->setProto(unisolatedProto.release());
  }
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  // 7. Toggle Blend enabled on a 2-blend structure -> compiles unseen, then
  // disabling reuses the 1-blend cache and re-enabling reuses the 2-blend
  // cache.
  system->addBlend();
  auto* blend1 = system->blends()->at(1);
  blend1->addFormula("Swirl");
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), ++expectedCompiles);

  blend1->setEnabled(false);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);

  blend1->setEnabled(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_EQ(view_->compiledPipelineCount(), expectedCompiles);
}

TEST_F(
    SystemViewTest,
    AdaptiveDispatchAccumulatesWithoutTimerAndPausedColorMapEditClearsBlack) {
  QTemporaryDir tempDir;
  ASSERT_TRUE(tempDir.isValid());

  // 64x64 at 150 IPP = 614,400 iterations, requiring multiple adaptive frames
  // driven solely by QQuickRhiItemRenderer::update() without any QML Timer.
  projectModel_->setWidth(64);
  projectModel_->setHeight(64);
  view_->setTargetIterationsPerPixel(150.0);

  QSignalSpy frameSpy(window_.get(), &QQuickWindow::frameSwapped);
  const int initialFrames = frameSpy.size();

  view_->setRunning(true);
  ASSERT_TRUE(QTest::qWaitFor([&]() { return view_->converged(); }, 5000));
  EXPECT_GT(frameSpy.size(), initialFrames);
  EXPECT_GE(view_->currentIterationsPerPixel(), 150.0);

  const quint64 compilesAfterRun = view_->compiledPipelineCount();

  // Pause and switch ColorMap: must clear HistogramSSBO + CounterSSBO to black
  // without compiling shaders or stepping particles.
  view_->setRunning(false);
  projectModel_->colorMap()->setName("Rainbow");
  EXPECT_EQ(view_->plottedIterations(), 0u);
  EXPECT_DOUBLE_EQ(view_->currentIterationsPerPixel(), 0.0);

  const QImage clearedImg =
      exportProjectImage(view_, tempDir.filePath("paused_colormap_clear.png"));
  ASSERT_FALSE(clearedImg.isNull());
  EXPECT_EQ(clearedImg.size(), QSize(64, 64));
  for (int y = 0; y < clearedImg.height(); ++y) {
    for (int x = 0; x < clearedImg.width(); ++x) {
      const QColor px = clearedImg.pixelColor(x, y);
      EXPECT_EQ(px.red(), 0);
      EXPECT_EQ(px.green(), 0);
      EXPECT_EQ(px.blue(), 0);
    }
  }
  EXPECT_EQ(view_->plottedIterations(), 0u);
  EXPECT_EQ(view_->compiledPipelineCount(), compilesAfterRun);
}

}  // namespace
}  // namespace chaoskit::ui

int main(int argc, char** argv) {
  QGuiApplication app(argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
