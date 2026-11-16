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

}  // namespace
}  // namespace chaoskit::ui

int main(int argc, char** argv) {
  QGuiApplication app(argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
