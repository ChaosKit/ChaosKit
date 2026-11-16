#include <gmock/gmock.h>
#include <google/protobuf/text_format.h>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QProcess>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>

#include "chaoskit.pb.h"

#ifndef CHAOSKIT_RENDER_BIN
#error "CHAOSKIT_RENDER_BIN must be defined"
#endif

#ifndef CHAOSKIT_TEST_DATA_DIR
#error "CHAOSKIT_TEST_DATA_DIR must be defined"
#endif

namespace chaoskit::cli {
namespace {

using ::testing::HasSubstr;

struct RunResult {
  int exitCode;
  QProcess::ExitStatus exitStatus;
  QString stdOut;
  QString stdErr;
};

RunResult runChaoskitRender(const QStringList& args) {
  QProcess process;
  process.start(QString::fromUtf8(CHAOSKIT_RENDER_BIN), args);
  bool finished = process.waitForFinished(30000);
  if (!finished) {
    process.kill();
    return {-1, QProcess::CrashExit, "", "Process timed out"};
  }
  return {
      process.exitCode(),
      process.exitStatus(),
      QString::fromUtf8(process.readAllStandardOutput()),
      QString::fromUtf8(process.readAllStandardError()),
  };
}

double calculateNormalizedRmse(const QImage& img1, const QImage& img2,
                               QImage* diffImage) {
  if (img1.size() != img2.size()) {
    return 1.0;
  }

  QImage a = img1.convertToFormat(QImage::Format_RGBA8888);
  QImage b = img2.convertToFormat(QImage::Format_RGBA8888);

  int width = a.width();
  int height = a.height();
  if (diffImage != nullptr) {
    *diffImage = QImage(width, height, QImage::Format_RGBA8888);
  }

  double sumSquaredError = 0.0;
  size_t totalChannels = static_cast<size_t>(width) * height * 3;

  for (int y = 0; y < height; ++y) {
    const uchar* lineA = a.constScanLine(y);
    const uchar* lineB = b.constScanLine(y);
    uchar* lineDiff = diffImage != nullptr ? diffImage->scanLine(y) : nullptr;

    for (int x = 0; x < width; ++x) {
      int idx = x * 4;
      double diffR =
          (static_cast<double>(lineA[idx + 0]) - lineB[idx + 0]) / 255.0;
      double diffG =
          (static_cast<double>(lineA[idx + 1]) - lineB[idx + 1]) / 255.0;
      double diffB =
          (static_cast<double>(lineA[idx + 2]) - lineB[idx + 2]) / 255.0;

      sumSquaredError += diffR * diffR + diffG * diffG + diffB * diffB;

      if (lineDiff != nullptr) {
        lineDiff[idx + 0] = static_cast<uchar>(
            std::clamp(std::abs(lineA[idx + 0] - lineB[idx + 0]), 0, 255));
        lineDiff[idx + 1] = static_cast<uchar>(
            std::clamp(std::abs(lineA[idx + 1] - lineB[idx + 1]), 0, 255));
        lineDiff[idx + 2] = static_cast<uchar>(
            std::clamp(std::abs(lineA[idx + 2] - lineB[idx + 2]), 0, 255));
        lineDiff[idx + 3] = 255;
      }
    }
  }

  return std::sqrt(sumSquaredError / static_cast<double>(totalChannels));
}

QByteArray readFileBytes(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    ADD_FAILURE() << "Failed to open file: " << path.toStdString();
    return {};
  }
  return file.readAll();
}

void writeTextFile(const QString& path, const char* content) {
  QFile file(path);
  bool opened = file.open(QIODevice::WriteOnly | QIODevice::Text);
  ASSERT_TRUE(opened) << "Failed to write file: " << path.toStdString();
  file.write(content);
  file.close();
}

class CliTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
#if !defined(Q_OS_MACOS)
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM") &&
        !qEnvironmentVariableIsSet("DISPLAY") &&
        !qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
      qputenv("QT_QPA_PLATFORM", "offscreen");
    }
#endif
    if (QCoreApplication::instance() == nullptr) {
      static int argc = 1;
      static char name[] = "cli_test";
      static char* argv[] = {name, nullptr};
      static QGuiApplication app(argc, argv);
    }
  }

  void SetUp() override {
    testDataDir_ = QString::fromUtf8(CHAOSKIT_TEST_DATA_DIR);
    ASSERT_TRUE(tempDir_.isValid());
  }

  QString testDataDir_;
  QTemporaryDir tempDir_;
};

TEST_F(CliTest, ErrorWhenFileCannotBeReadOrParsed) {
  QString outPath = tempDir_.filePath("out.png");

  // Non-existent file
  RunResult resMissing =
      runChaoskitRender({"/non/existent/path.pbtxt", outPath});
  EXPECT_NE(resMissing.exitCode, 0);
  EXPECT_FALSE(resMissing.stdErr.isEmpty());

  // Unparseable file
  QTemporaryFile badFile;
  bool badOpened = badFile.open();
  ASSERT_TRUE(badOpened);
  badFile.write("this is not a valid project protobuf content");
  badFile.close();

  RunResult resBad = runChaoskitRender({badFile.fileName(), outPath});
  EXPECT_NE(resBad.exitCode, 0);
  EXPECT_FALSE(resBad.stdErr.isEmpty());
}

TEST_F(CliTest, ErrorOnUnrecognizedColorMap) {
  QString inputPath = testDataDir_ + "/invalid_colormap.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  RunResult res = runChaoskitRender({inputPath, outPath});
  EXPECT_NE(res.exitCode, 0);
  EXPECT_THAT(res.stdErr.toStdString(), HasSubstr("NonExistentColorMap"));
}

TEST_F(CliTest, WarningOutputPrintedToStderr) {
  QString inputPath = testDataDir_ + "/warnings.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  RunResult res = runChaoskitRender({inputPath, outPath, "--seed", "42"});
  EXPECT_EQ(res.exitCode, 0);
  EXPECT_THAT(res.stdErr.toStdString(), HasSubstr("NonExistentFormulaType"));
  EXPECT_THAT(res.stdErr.toStdString(), HasSubstr("isolated blend"));
}

TEST_F(CliTest, BitwiseDeterminismWithSameSeed) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath1 = tempDir_.filePath("run1.png");
  QString outPath2 = tempDir_.filePath("run2.png");

  RunResult res1 = runChaoskitRender({inputPath, outPath1, "--seed", "42"});
  EXPECT_EQ(res1.exitCode, 0);

  RunResult res2 = runChaoskitRender({inputPath, outPath2, "--seed", "42"});
  EXPECT_EQ(res2.exitCode, 0);

  QFile file1(outPath1);
  bool opened1 = file1.open(QIODevice::ReadOnly);
  ASSERT_TRUE(opened1);
  QByteArray bytes1 = file1.readAll();

  QFile file2(outPath2);
  bool opened2 = file2.open(QIODevice::ReadOnly);
  ASSERT_TRUE(opened2);
  QByteArray bytes2 = file2.readAll();

  EXPECT_GT(bytes1.size(), 0);
  EXPECT_EQ(bytes1, bytes2);
}

TEST_F(CliTest, GoldenImageVerification) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString cpuPath = tempDir_.filePath("simple_cpu_actual.png");
  QString auto100Path = tempDir_.filePath("simple_auto_100.png");
  QString auto400Path = tempDir_.filePath("simple_auto_400.png");
  QString goldenPath = testDataDir_ + "/simple_golden.png";

  // Legacy golden compatibility: --backend cpu --seed 42 uses pcg32(42).
  RunResult cpuRes = runChaoskitRender(
      {inputPath, cpuPath, "--backend", "cpu", "--seed", "42"});
  ASSERT_EQ(cpuRes.exitCode, 0) << cpuRes.stdErr.toStdString();

  bool updateGoldens = qEnvironmentVariableIsSet("CHAOSKIT_UPDATE_GOLDENS") &&
                       qEnvironmentVariable("CHAOSKIT_UPDATE_GOLDENS") != "0";
  if (updateGoldens) {
    if (QFile::exists(goldenPath)) {
      QFile::remove(goldenPath);
    }
    ASSERT_TRUE(QFile::copy(cpuPath, goldenPath));
  }

  QImage cpuImage(cpuPath);
  ASSERT_FALSE(cpuImage.isNull())
      << "Failed to load CPU actual image: " << cpuPath.toStdString();

  QImage goldenImage(goldenPath);
  if (goldenImage.isNull()) {
    cpuImage.save("simple_actual.png");
    FAIL() << "Golden image missing or invalid at " << goldenPath.toStdString()
           << ". Set CHAOSKIT_UPDATE_GOLDENS=1 to generate.";
  }

  QImage diffImage;
  double cpuRmse = calculateNormalizedRmse(cpuImage, goldenImage, &diffImage);
  if (cpuRmse > 0.05) {
    cpuImage.save("simple_actual.png");
    diffImage.save("simple_diff.png");
  }
  EXPECT_LE(cpuRmse, 0.05);

  // Default (--backend auto) with --seed 42 runs on GPU (Philox); verify
  // statistical parity at 100 IPP (<= 0.06) and strict convergence at 400 IPP
  // (<= 0.05).
  RunResult auto100Res =
      runChaoskitRender({inputPath, auto100Path, "--seed", "42"});
  ASSERT_EQ(auto100Res.exitCode, 0) << auto100Res.stdErr.toStdString();
  QImage auto100Image(auto100Path);
  ASSERT_FALSE(auto100Image.isNull());
  EXPECT_LE(calculateNormalizedRmse(auto100Image, goldenImage, nullptr), 0.06);

  RunResult auto400Res = runChaoskitRender(
      {inputPath, auto400Path, "--seed", "42", "--ipp", "400"});
  ASSERT_EQ(auto400Res.exitCode, 0) << auto400Res.stdErr.toStdString();
  QImage auto400Image(auto400Path);
  ASSERT_FALSE(auto400Image.isNull());
  EXPECT_LE(calculateNormalizedRmse(auto400Image, goldenImage, nullptr), 0.05);
}

TEST_F(CliTest, ErrorWhenBothIppAndIterationsPassed) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  // Passing --ipp and --iterations
  RunResult res1 = runChaoskitRender(
      {inputPath, outPath, "--ipp", "100", "--iterations", "1000"});
  EXPECT_NE(res1.exitCode, 0);
  EXPECT_FALSE(res1.stdErr.isEmpty());

  // Passing --iterations-per-pixel and --iterations
  RunResult res2 =
      runChaoskitRender({inputPath, outPath, "--iterations-per-pixel", "100",
                         "--iterations", "1000"});
  EXPECT_NE(res2.exitCode, 0);
  EXPECT_FALSE(res2.stdErr.isEmpty());
}

TEST_F(CliTest, ErrorWhenSeedAndThreadsGreaterThanOnePassed) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  RunResult res =
      runChaoskitRender({inputPath, outPath, "--seed", "42", "--threads", "2"});
  EXPECT_NE(res.exitCode, 0);
  EXPECT_FALSE(res.stdErr.isEmpty());

  // --seed with --threads 1 is allowed
  RunResult resSingle =
      runChaoskitRender({inputPath, outPath, "--seed", "42", "--threads", "1"});
  EXPECT_EQ(resSingle.exitCode, 0);
}

TEST_F(CliTest, ErrorOnInvalidWorkloadOrConcurrencyFlags) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  // Invalid --threads
  for (const QString& val : {"0", "-1", "foo", "1.5"}) {
    RunResult res = runChaoskitRender({inputPath, outPath, "--threads", val});
    EXPECT_NE(res.exitCode, 0)
        << "Expected failure for --threads " << val.toStdString();
    EXPECT_FALSE(res.stdErr.isEmpty());
  }

  // Invalid --ipp
  for (const QString& val : {"0", "-5", "bar", "2.5"}) {
    RunResult res = runChaoskitRender({inputPath, outPath, "--ipp", val});
    EXPECT_NE(res.exitCode, 0)
        << "Expected failure for --ipp " << val.toStdString();
    EXPECT_FALSE(res.stdErr.isEmpty());
  }

  // Invalid --iterations-per-pixel
  {
    RunResult res =
        runChaoskitRender({inputPath, outPath, "--iterations-per-pixel", "0"});
    EXPECT_NE(res.exitCode, 0);
    EXPECT_FALSE(res.stdErr.isEmpty());
  }

  // Invalid --iterations
  for (const QString& val : {"0", "-10", "baz", "3.14"}) {
    RunResult res =
        runChaoskitRender({inputPath, outPath, "--iterations", val});
    EXPECT_NE(res.exitCode, 0)
        << "Expected failure for --iterations " << val.toStdString();
    EXPECT_FALSE(res.stdErr.isEmpty());
  }
}

TEST_F(CliTest, WorkloadFlagsExecution) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPathIpp = tempDir_.filePath("out_ipp.png");
  QString outPathIppLong = tempDir_.filePath("out_ipp_long.png");
  QString outPathIter = tempDir_.filePath("out_iter.png");

  RunResult resIpp =
      runChaoskitRender({inputPath, outPathIpp, "--seed", "42", "--ipp", "50"});
  EXPECT_EQ(resIpp.exitCode, 0);
  QImage imgIpp(outPathIpp);
  EXPECT_FALSE(imgIpp.isNull());

  RunResult resIppLong =
      runChaoskitRender({inputPath, outPathIppLong, "--seed", "42",
                         "--iterations-per-pixel", "50"});
  EXPECT_EQ(resIppLong.exitCode, 0);
  QImage imgIppLong(outPathIppLong);
  EXPECT_FALSE(imgIppLong.isNull());

  RunResult resIter = runChaoskitRender(
      {inputPath, outPathIter, "--seed", "42", "--iterations", "10000"});
  EXPECT_EQ(resIter.exitCode, 0);
  QImage imgIter(outPathIter);
  EXPECT_FALSE(imgIter.isNull());
}

TEST_F(CliTest, MultiThreadedRenderingWithoutSeedMatchesGoldenImage) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString actualPath = tempDir_.filePath("simple_multithreaded.png");
  QString goldenPath = testDataDir_ + "/simple_golden.png";

  RunResult res = runChaoskitRender({inputPath, actualPath, "--threads", "2"});
  EXPECT_EQ(res.exitCode, 0);

  QImage actualImage(actualPath);
  ASSERT_FALSE(actualImage.isNull());

  QImage goldenImage(goldenPath);
  ASSERT_FALSE(goldenImage.isNull());

  QImage diffImage;
  double rmse = calculateNormalizedRmse(actualImage, goldenImage, &diffImage);
  // Two independent 100-IPP 64x64 Monte Carlo runs (~0.046 RMSE) combined in
  // quadrature with fixing GathererTask's negative-coordinate truncation on
  // border pixels (~0.0215 RMSE against simple_golden.png) yield ~0.0505.
  EXPECT_LE(rmse, 0.06);
}

TEST_F(CliTest, ErrorOnInvalidWidthOrHeight) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  for (const char* val : {"0", "abc", "-10"}) {
    RunResult resWidth =
        runChaoskitRender({inputPath, outPath, "--width", val});
    EXPECT_NE(resWidth.exitCode, 0) << "Expected failure for --width " << val;
    EXPECT_THAT(resWidth.stdErr.toStdString(), HasSubstr("width"));

    RunResult resHeight =
        runChaoskitRender({inputPath, outPath, "--height", val});
    EXPECT_NE(resHeight.exitCode, 0) << "Expected failure for --height " << val;
    EXPECT_THAT(resHeight.stdErr.toStdString(), HasSubstr("height"));
  }
}

TEST_F(CliTest, ErrorOnNonPositiveGamma) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  for (const char* val : {"0", "0.0", "-1.0", "abc"}) {
    RunResult res = runChaoskitRender({inputPath, outPath, "--gamma", val});
    EXPECT_NE(res.exitCode, 0) << "Expected failure for --gamma " << val;
    EXPECT_THAT(res.stdErr.toStdString(), HasSubstr("gamma"));
  }
}

TEST_F(CliTest, ErrorOnMalformedExposureOrVibrancy) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  RunResult resExp =
      runChaoskitRender({inputPath, outPath, "--exposure", "abc"});
  EXPECT_NE(resExp.exitCode, 0);
  EXPECT_THAT(resExp.stdErr.toStdString(), HasSubstr("exposure"));

  RunResult resVib =
      runChaoskitRender({inputPath, outPath, "--vibrancy", "abc"});
  EXPECT_NE(resVib.exitCode, 0);
  EXPECT_THAT(resVib.stdErr.toStdString(), HasSubstr("vibrancy"));
}

TEST_F(CliTest, ErrorOnUnregisteredColorMapOverride) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  RunResult res =
      runChaoskitRender({inputPath, outPath, "--color-map", "NonExistentMap"});
  EXPECT_NE(res.exitCode, 0);
  EXPECT_THAT(res.stdErr.toStdString(), HasSubstr("NonExistentMap"));
}

TEST_F(CliTest, OverrideDimensions) {
  QString inputPath = tempDir_.filePath("dim_test.pbtxt");
  QFile file(inputPath);
  bool opened = file.open(QIODevice::WriteOnly | QIODevice::Text);
  ASSERT_TRUE(opened);
  file.write("width: 64\nheight: 64\n");
  file.close();

  QString outPath = tempDir_.filePath("out_dim.png");

  RunResult res = runChaoskitRender({inputPath, outPath, "--width", "512",
                                     "--height", "256", "--seed", "42"});
  EXPECT_EQ(res.exitCode, 0);

  QImage image(outPath);
  ASSERT_FALSE(image.isNull());
  EXPECT_EQ(image.width(), 512);
  EXPECT_EQ(image.height(), 256);
}

TEST_F(CliTest, OverrideToneMappingParameters) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString basePath = tempDir_.filePath("base.png");
  QString gammaPath = tempDir_.filePath("gamma.png");
  QString exposurePath = tempDir_.filePath("exposure.png");
  QString vibrancyPath = tempDir_.filePath("vibrancy.png");

  RunResult resBase = runChaoskitRender({inputPath, basePath, "--seed", "42"});
  ASSERT_EQ(resBase.exitCode, 0);
  QImage baseImg(basePath);
  ASSERT_FALSE(baseImg.isNull());

  // Test --gamma override
  RunResult resGamma = runChaoskitRender(
      {inputPath, gammaPath, "--seed", "42", "--gamma", "1.0"});
  ASSERT_EQ(resGamma.exitCode, 0);
  QImage gammaImg(gammaPath);
  ASSERT_FALSE(gammaImg.isNull());
  EXPECT_GT(calculateNormalizedRmse(baseImg, gammaImg, nullptr), 0.01);

  // Test --exposure override
  RunResult resExp = runChaoskitRender(
      {inputPath, exposurePath, "--seed", "42", "--exposure", "2.0"});
  ASSERT_EQ(resExp.exitCode, 0);
  QImage expImg(exposurePath);
  ASSERT_FALSE(expImg.isNull());
  EXPECT_GT(calculateNormalizedRmse(baseImg, expImg, nullptr), 0.01);

  // Test --vibrancy override
  RunResult resVib = runChaoskitRender(
      {inputPath, vibrancyPath, "--seed", "42", "--vibrancy", "1.0"});
  ASSERT_EQ(resVib.exitCode, 0);
  QImage vibImg(vibrancyPath);
  ASSERT_FALSE(vibImg.isNull());
  EXPECT_GT(calculateNormalizedRmse(baseImg, vibImg, nullptr), 0.01);
}

TEST_F(CliTest, OverrideColorMap) {
  // Create a project without color_map specified (defaults to BGR)
  QString bgrProjectPath = tempDir_.filePath("default_bgr.pbtxt");
  QFile bgrFile(bgrProjectPath);
  bool bgrOpened = bgrFile.open(QIODevice::WriteOnly | QIODevice::Text);
  ASSERT_TRUE(bgrOpened);
  bgrFile.write(R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
system {
  min_lifetime: 1
  max_lifetime: 10
  blends {
    enabled: true
    weight: 1.0
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight {
        x: 1.0
      }
    }
    coloring_method {
      single_color: 0.5
    }
  }
}
)");
  bgrFile.close();

  QString bgrOut = tempDir_.filePath("bgr.png");
  QString rainbowOut = tempDir_.filePath("rainbow.png");

  RunResult resBgr =
      runChaoskitRender({bgrProjectPath, bgrOut, "--seed", "42"});
  ASSERT_EQ(resBgr.exitCode, 0);
  QImage bgrImg(bgrOut);
  ASSERT_FALSE(bgrImg.isNull());

  RunResult resRainbow = runChaoskitRender(
      {bgrProjectPath, rainbowOut, "--seed", "42", "--color-map", "Rainbow"});
  ASSERT_EQ(resRainbow.exitCode, 0);
  QImage rainbowImg(rainbowOut);
  ASSERT_FALSE(rainbowImg.isNull());

  EXPECT_GT(calculateNormalizedRmse(bgrImg, rainbowImg, nullptr), 0.05);
}

TEST_F(CliTest, BinaryProjectFileRendering) {
  QString inputPath = testDataDir_ + "/simple.ck";
  QString pbtxtPath = testDataDir_ + "/simple.pbtxt";
  QString goldenPath = testDataDir_ + "/simple_golden.png";
  QString ckCpuPath = tempDir_.filePath("simple_ck_cpu.png");
  QString pbtxtCpuPath = tempDir_.filePath("simple_pbtxt_cpu.png");
  QString ckAutoPath = tempDir_.filePath("simple_ck_auto.png");
  QString ckAuto400Path = tempDir_.filePath("simple_ck_auto_400.png");
  QString pbtxtAutoPath = tempDir_.filePath("simple_pbtxt_auto.png");

  QImage goldenImage(goldenPath);
  ASSERT_FALSE(goldenImage.isNull());

  ASSERT_EQ(runChaoskitRender(
                {inputPath, ckCpuPath, "--backend", "cpu", "--seed", "42"})
                .exitCode,
            0);
  ASSERT_EQ(runChaoskitRender(
                {pbtxtPath, pbtxtCpuPath, "--backend", "cpu", "--seed", "42"})
                .exitCode,
            0);
  QImage ckCpuImage(ckCpuPath);
  ASSERT_FALSE(ckCpuImage.isNull());
  EXPECT_LE(calculateNormalizedRmse(ckCpuImage, goldenImage, nullptr), 0.05);
  EXPECT_EQ(readFileBytes(ckCpuPath), readFileBytes(pbtxtCpuPath));

  // Default (--backend auto) renders .ck and .pbtxt identically on GPU.
  ASSERT_EQ(runChaoskitRender({inputPath, ckAutoPath, "--seed", "42"}).exitCode,
            0);
  ASSERT_EQ(
      runChaoskitRender({pbtxtPath, pbtxtAutoPath, "--seed", "42"}).exitCode,
      0);
  QImage ckAutoImage(ckAutoPath);
  ASSERT_FALSE(ckAutoImage.isNull());
  EXPECT_LE(calculateNormalizedRmse(ckAutoImage, goldenImage, nullptr), 0.06);
  EXPECT_EQ(readFileBytes(ckAutoPath), readFileBytes(pbtxtAutoPath));

  ASSERT_EQ(runChaoskitRender(
                {inputPath, ckAuto400Path, "--seed", "42", "--ipp", "400"})
                .exitCode,
            0);
  QImage ckAuto400Image(ckAuto400Path);
  ASSERT_FALSE(ckAuto400Image.isNull());
  EXPECT_LE(calculateNormalizedRmse(ckAuto400Image, goldenImage, nullptr),
            0.05);
}

TEST_F(CliTest, ExtensionBasedDispatchAndFallback) {
  // Read a valid project text proto
  QFile pbtxtFile(testDataDir_ + "/simple.pbtxt");
  bool pbtxtReadOpened = pbtxtFile.open(QIODevice::ReadOnly | QIODevice::Text);
  ASSERT_TRUE(pbtxtReadOpened);
  QByteArray textProto = pbtxtFile.readAll();
  pbtxtFile.close();

  // Read a valid binary project proto
  QFile ckFile(testDataDir_ + "/simple.ck");
  bool ckReadOpened = ckFile.open(QIODevice::ReadOnly);
  ASSERT_TRUE(ckReadOpened);
  QByteArray binaryProto = ckFile.readAll();
  ckFile.close();

  // 1. Text proto saved as .pbtxt (primary dispatch succeeds)
  {
    QString path = tempDir_.filePath("test_valid.pbtxt");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write(textProto);
    f.close();
    QString out = tempDir_.filePath("out_valid_pbtxt.png");
    RunResult res = runChaoskitRender({path, out, "--seed", "42"});
    EXPECT_EQ(res.exitCode, 0);
  }

  // 2. Text proto saved as .cktxt (primary dispatch succeeds)
  {
    QString path = tempDir_.filePath("test_valid.cktxt");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write(textProto);
    f.close();
    QString out = tempDir_.filePath("out_valid_cktxt.png");
    RunResult res = runChaoskitRender({path, out, "--seed", "42"});
    EXPECT_EQ(res.exitCode, 0);
  }

  // 3. Binary proto saved as .ck (primary dispatch succeeds)
  {
    QString path = tempDir_.filePath("test_valid.ck");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write(binaryProto);
    f.close();
    QString out = tempDir_.filePath("out_valid_ck.png");
    RunResult res = runChaoskitRender({path, out, "--seed", "42"});
    EXPECT_EQ(res.exitCode, 0);
  }

  // 4. Binary proto saved with .pbtxt extension (fallback: text fails, binary
  // succeeds)
  {
    QString path = tempDir_.filePath("binary_named.pbtxt");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write(binaryProto);
    f.close();
    QString out = tempDir_.filePath("out_bin_as_pbtxt.png");
    RunResult res = runChaoskitRender({path, out, "--seed", "42"});
    EXPECT_EQ(res.exitCode, 0);
  }

  // 5. Binary proto saved with .cktxt extension (fallback: text fails, binary
  // succeeds)
  {
    QString path = tempDir_.filePath("binary_named.cktxt");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write(binaryProto);
    f.close();
    QString out = tempDir_.filePath("out_bin_as_cktxt.png");
    RunResult res = runChaoskitRender({path, out, "--seed", "42"});
    EXPECT_EQ(res.exitCode, 0);
  }

  // 6. Text proto saved with .ck extension (fallback: binary fails, text
  // succeeds)
  {
    QString path = tempDir_.filePath("text_named.ck");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write(textProto);
    f.close();
    QString out = tempDir_.filePath("out_text_as_ck.png");
    RunResult res = runChaoskitRender({path, out, "--seed", "42"});
    EXPECT_EQ(res.exitCode, 0);
  }

  // 7. Text proto saved with arbitrary extension .foo (fallback: binary fails,
  // text succeeds)
  {
    QString path = tempDir_.filePath("text_named.foo");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write(textProto);
    f.close();
    QString out = tempDir_.filePath("out_text_as_foo.png");
    RunResult res = runChaoskitRender({path, out, "--seed", "42"});
    EXPECT_EQ(res.exitCode, 0);
  }

  // 8. Corrupt file with .pbtxt extension (both fail -> returns 1 with error)
  {
    QString path = tempDir_.filePath("corrupt.pbtxt");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write("not a valid proto text or binary data");
    f.close();
    QString out = tempDir_.filePath("out_corrupt_pbtxt.png");
    RunResult res = runChaoskitRender({path, out});
    EXPECT_NE(res.exitCode, 0);
    EXPECT_THAT(res.stdErr.toStdString(),
                HasSubstr("Failed to parse project file"));
  }

  // 9. Corrupt file with .ck extension (both fail -> returns 1 with error)
  {
    QString path = tempDir_.filePath("corrupt.ck");
    QFile f(path);
    bool opened = f.open(QIODevice::WriteOnly);
    ASSERT_TRUE(opened);
    f.write("not a valid proto text or binary data");
    f.close();
    QString out = tempDir_.filePath("out_corrupt_ck.png");
    RunResult res = runChaoskitRender({path, out});
    EXPECT_NE(res.exitCode, 0);
    EXPECT_THAT(res.stdErr.toStdString(),
                HasSubstr("Failed to parse project file"));
  }
}

TEST_F(CliTest, MultiBlendAndCameraRendering) {
  QString inputPath = testDataDir_ + "/multi_blend_camera.pbtxt";
  QString outPath = tempDir_.filePath("multi_blend_out.png");

  RunResult res = runChaoskitRender({inputPath, outPath, "--seed", "42"});
  EXPECT_EQ(res.exitCode, 0);

  QImage image(outPath);
  ASSERT_FALSE(image.isNull());
  EXPECT_EQ(image.width(), 64);
  EXPECT_EQ(image.height(), 64);
}

struct LuminanceStats {
  double meanLuminance = 0.0;
  double whiteFraction = 0.0;
  size_t opaqueBlackPixels = 0;
  bool allOpaque = true;
};

LuminanceStats analyzeLuminance(const QImage& img) {
  QImage rgba = img.convertToFormat(QImage::Format_RGBA8888);
  const int width = rgba.width();
  const int height = rgba.height();
  const size_t pixelCount = static_cast<size_t>(width) * height;
  if (pixelCount == 0) {
    return {};
  }

  double lumSum = 0.0;
  size_t whiteCount = 0;
  LuminanceStats stats;
  for (int y = 0; y < height; ++y) {
    const uchar* line = rgba.constScanLine(y);
    for (int x = 0; x < width; ++x) {
      const int idx = x * 4;
      const uchar r = line[idx + 0];
      const uchar g = line[idx + 1];
      const uchar b = line[idx + 2];
      const uchar a = line[idx + 3];
      if (a != 255) {
        stats.allOpaque = false;
      }
      if (r == 0 && g == 0 && b == 0 && a == 255) {
        ++stats.opaqueBlackPixels;
      }
      if (r >= 250 && g >= 250 && b >= 250) {
        ++whiteCount;
      }
      const double nr = static_cast<double>(r) / 255.0;
      const double ng = static_cast<double>(g) / 255.0;
      const double nb = static_cast<double>(b) / 255.0;
      lumSum += 0.2126 * nr + 0.7152 * ng + 0.0722 * nb;
    }
  }
  stats.meanLuminance = lumSum / static_cast<double>(pixelCount);
  stats.whiteFraction =
      static_cast<double>(whiteCount) / static_cast<double>(pixelCount);
  return stats;
}

TEST_F(CliTest, DensityNormalizedLuminanceAcrossSampleDensities) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString out5Path = tempDir_.filePath("ipp_5.png");
  QString out100Path = tempDir_.filePath("ipp_100.png");
  QString out2000Path = tempDir_.filePath("ipp_2000.png");

  ASSERT_EQ(runChaoskitRender({inputPath, out5Path, "--width", "32", "--height",
                               "32", "--seed", "42", "--ipp", "5"})
                .exitCode,
            0);
  ASSERT_EQ(
      runChaoskitRender({inputPath, out100Path, "--width", "32", "--height",
                         "32", "--seed", "42", "--ipp", "100"})
          .exitCode,
      0);
  ASSERT_EQ(
      runChaoskitRender({inputPath, out2000Path, "--width", "32", "--height",
                         "32", "--seed", "42", "--ipp", "2000"})
          .exitCode,
      0);

  QImage img5(out5Path);
  QImage img100(out100Path);
  QImage img2000(out2000Path);
  ASSERT_FALSE(img5.isNull());
  ASSERT_FALSE(img100.isNull());
  ASSERT_FALSE(img2000.isNull());

  const LuminanceStats stats5 = analyzeLuminance(img5);
  const LuminanceStats stats100 = analyzeLuminance(img100);
  const LuminanceStats stats2000 = analyzeLuminance(img2000);

  EXPECT_TRUE(stats5.allOpaque);
  EXPECT_TRUE(stats100.allOpaque);
  EXPECT_TRUE(stats2000.allOpaque);

  // Baseline 100 IPP should have meaningful brightness without clipping white.
  EXPECT_GT(stats100.meanLuminance, 0.02);
  EXPECT_LT(stats100.meanLuminance, 0.70);

  // Low density (5 IPP) should preserve overall brightness without blacking
  // out.
  EXPECT_GE(stats5.meanLuminance, 0.35 * stats100.meanLuminance);
  EXPECT_LE(stats5.meanLuminance, 1.50 * stats100.meanLuminance);

  // Deep density (2000 IPP) should maintain consistent brightness without
  // blowing out to white.
  EXPECT_GE(stats2000.meanLuminance, 0.85 * stats100.meanLuminance);
  EXPECT_LE(stats2000.meanLuminance, 1.15 * stats100.meanLuminance);
  EXPECT_LE(stats2000.whiteFraction, stats100.whiteFraction + 0.01);
  EXPECT_LE(calculateNormalizedRmse(img100, img2000, nullptr), 0.08);
}

TEST_F(CliTest, OpaqueBlackWhenZeroPlottedOrZeroHits) {
  // 1. Degenerate zero-scaled Spherical system hits 0/0 = NaN every tick,
  // producing plottedIterations == 0 (currentIpp == 0.0).
  {
    QString zeroIppPath = tempDir_.filePath("zero_ipp.pbtxt");
    QFile file(zeroIppPath);
    bool opened = file.open(QIODevice::WriteOnly | QIODevice::Text);
    ASSERT_TRUE(opened);
    file.write(R"(
width: 32
height: 32
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "BGR" }
system {
  min_lifetime: 0
  max_lifetime: 1
  blends {
    enabled: true
    weight: 1.0
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.5 }
  }
}
)");
    file.close();

    QString outPath = tempDir_.filePath("zero_ipp.png");
    RunResult res =
        runChaoskitRender({zeroIppPath, outPath, "--iterations", "10"});
    ASSERT_EQ(res.exitCode, 0);

    QImage img(outPath);
    ASSERT_FALSE(img.isNull());
    EXPECT_EQ(img.width(), 32);
    EXPECT_EQ(img.height(), 32);
    const LuminanceStats stats = analyzeLuminance(img);
    EXPECT_TRUE(stats.allOpaque);
    EXPECT_EQ(stats.opaqueBlackPixels, 32u * 32u);
  }

  // 2. Finite points translated off-canvas yield currentIpp > 0 with zero hits
  // in every pixel.
  {
    QString zeroHitsPath = tempDir_.filePath("zero_hits.pbtxt");
    QFile file(zeroHitsPath);
    bool opened = file.open(QIODevice::WriteOnly | QIODevice::Text);
    ASSERT_TRUE(opened);
    file.write(R"(
width: 32
height: 32
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "BGR" }
system {
  min_lifetime: 0
  max_lifetime: 5
  blends {
    enabled: true
    weight: 1.0
    formulas {
      type: "Linear"
      weight { x: 1.0 y: 1.0 }
    }
    post { m11: 1.0 m22: 1.0 m31: 100.0 }
    coloring_method { single_color: 0.5 }
  }
}
)");
    file.close();

    QString outPath = tempDir_.filePath("zero_hits.png");
    RunResult res = runChaoskitRender(
        {zeroHitsPath, outPath, "--seed", "42", "--ipp", "10"});
    ASSERT_EQ(res.exitCode, 0);

    QImage img(outPath);
    ASSERT_FALSE(img.isNull());
    const LuminanceStats stats = analyzeLuminance(img);
    EXPECT_TRUE(stats.allOpaque);
    EXPECT_EQ(stats.opaqueBlackPixels, 32u * 32u);
  }
}

TEST_F(CliTest, ErrorOnInvalidBackendOrGpuWithThreads) {
  QString inputPath = testDataDir_ + "/simple.pbtxt";
  QString outPath = tempDir_.filePath("out.png");

  RunResult resInvalid =
      runChaoskitRender({inputPath, outPath, "--backend", "vulkan"});
  EXPECT_NE(resInvalid.exitCode, 0);
  EXPECT_THAT(resInvalid.stdErr.toStdString(), HasSubstr("Invalid --backend"));

  RunResult resGpuThreads1 = runChaoskitRender(
      {inputPath, outPath, "--backend", "gpu", "--threads", "1"});
  EXPECT_NE(resGpuThreads1.exitCode, 0);
  EXPECT_FALSE(resGpuThreads1.stdErr.isEmpty());

  RunResult resGpuThreads2 = runChaoskitRender(
      {inputPath, outPath, "--backend", "gpu", "--threads", "2"});
  EXPECT_NE(resGpuThreads2.exitCode, 0);
  EXPECT_FALSE(resGpuThreads2.stdErr.isEmpty());

  RunResult resGpuThreadsSeed =
      runChaoskitRender({inputPath, outPath, "--backend", "gpu", "--threads",
                         "2", "--seed", "42"});
  EXPECT_NE(resGpuThreadsSeed.exitCode, 0);
  EXPECT_FALSE(resGpuThreadsSeed.stdErr.isEmpty());
}

TEST_F(CliTest, BackendAndThreadsFlagCombinations) {
  const QString inputPath = testDataDir_ + "/simple.pbtxt";
  const QString cpuSeed1Path = tempDir_.filePath("cpu_seed_1.png");
  const QString cpuSeed2Path = tempDir_.filePath("cpu_seed_2.png");
  const QString autoThreads1SeedPath =
      tempDir_.filePath("auto_threads1_seed.png");
  const QString defaultThreads1SeedPath =
      tempDir_.filePath("default_threads1_seed.png");
  const QString autoThreads2Path = tempDir_.filePath("auto_threads2.png");
  const QString cpuThreads2Path = tempDir_.filePath("cpu_threads2.png");
  const QString badOutPath = tempDir_.filePath("bad_combo.png");

  // --backend cpu --seed preserves single-threaded CPU bitwise determinism.
  ASSERT_EQ(runChaoskitRender(
                {inputPath, cpuSeed1Path, "--backend", "cpu", "--seed", "42"})
                .exitCode,
            0);
  ASSERT_EQ(runChaoskitRender(
                {inputPath, cpuSeed2Path, "--backend", "cpu", "--seed", "42"})
                .exitCode,
            0);
  const QByteArray cpuBytes1 = readFileBytes(cpuSeed1Path);
  const QByteArray cpuBytes2 = readFileBytes(cpuSeed2Path);
  EXPECT_GT(cpuBytes1.size(), 0);
  EXPECT_EQ(cpuBytes1, cpuBytes2);

  // --threads 1 with --backend auto (explicit or default) selects CPU.
  ASSERT_EQ(runChaoskitRender({inputPath, autoThreads1SeedPath, "--backend",
                               "auto", "--threads", "1", "--seed", "42"})
                .exitCode,
            0);
  ASSERT_EQ(runChaoskitRender({inputPath, defaultThreads1SeedPath, "--threads",
                               "1", "--seed", "42"})
                .exitCode,
            0);
  EXPECT_EQ(readFileBytes(autoThreads1SeedPath), cpuBytes1);
  EXPECT_EQ(readFileBytes(defaultThreads1SeedPath), cpuBytes1);

  // --threads > 1 with --seed fails on auto and cpu.
  RunResult autoThreads2Seed =
      runChaoskitRender({inputPath, badOutPath, "--backend", "auto",
                         "--threads", "2", "--seed", "42"});
  EXPECT_NE(autoThreads2Seed.exitCode, 0);
  EXPECT_FALSE(autoThreads2Seed.stdErr.isEmpty());

  RunResult cpuThreads2Seed =
      runChaoskitRender({inputPath, badOutPath, "--backend", "cpu", "--threads",
                         "2", "--seed", "42"});
  EXPECT_NE(cpuThreads2Seed.exitCode, 0);
  EXPECT_FALSE(cpuThreads2Seed.stdErr.isEmpty());

  // Unseeded --threads 2 succeeds on both auto and cpu.
  ASSERT_EQ(runChaoskitRender({inputPath, autoThreads2Path, "--backend", "auto",
                               "--threads", "2"})
                .exitCode,
            0);
  ASSERT_EQ(runChaoskitRender({inputPath, cpuThreads2Path, "--backend", "cpu",
                               "--threads", "2"})
                .exitCode,
            0);
  EXPECT_FALSE(QImage(autoThreads2Path).isNull());
  EXPECT_FALSE(QImage(cpuThreads2Path).isNull());
}

TEST_F(CliTest, GpuVsCpuVisualParityAcrossFixtures) {
  struct FixtureCase {
    const char* name;
    const char* pbtxt;
    int ipp;
    double minMeanLuminance;
  };

  const FixtureCase cases[] = {
      {
          "ExPowNegativeBase",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "BGR" }
system {
  min_lifetime: 1
  max_lifetime: 12
  blends {
    enabled: true
    weight: 1.0
    pre { m11: 0.7 m22: 0.7 }
    formulas {
      type: "Ex"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.4 }
  }
}
)",
          400,
          0.01,
      },
      {
          "SphericalHyperbolicDistance",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "Rainbow" }
system {
  min_lifetime: 0
  max_lifetime: 20
  blends {
    enabled: true
    weight: 0.6
    pre { m11: 0.8 m22: 0.8 m31: 0.1 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { distance: 0.5 }
  }
  blends {
    enabled: true
    weight: 0.4
    pre { m11: 0.5 m22: 0.5 m32: -0.1 }
    formulas {
      type: "Hyperbolic"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.8 }
  }
}
)",
          400,
          0.01,
      },
      {
          "FiniteLifetimeSingularityWarmup",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "Rainbow" }
system {
  min_lifetime: 3
  max_lifetime: 15
  blends {
    enabled: true
    weight: 0.6
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.25 }
  }
  blends {
    enabled: true
    weight: 0.2
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.7 }
  }
  blends {
    enabled: true
    weight: 0.2
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Hyperbolic"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.9 }
  }
}
)",
          400,
          0.01,
      },
      {
          "InfiniteLifetimeSingularityRevival",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "Rainbow" }
system {
  min_lifetime: 2
  max_lifetime: -1
  blends {
    enabled: true
    weight: 0.6
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.3 }
  }
  blends {
    enabled: true
    weight: 0.2
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.7 }
  }
  blends {
    enabled: true
    weight: 0.2
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Hyperbolic"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.9 }
  }
}
)",
          400,
          0.01,
      },
      {
          "MultiBlendWeightsAndZeroWeightBranch",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "BGR" }
system {
  min_lifetime: 1
  max_lifetime: 12
  blends {
    enabled: true
    weight: 0.7
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.2 }
  }
  blends {
    enabled: true
    weight: 0.3
    pre { m11: 0.75 m22: 0.75 m31: 0.1 }
    formulas {
      type: "Swirl"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.8 }
  }
  blends {
    enabled: true
    weight: 0.0
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.5 }
  }
  blends {
    enabled: false
    weight: 10.0
    formulas {
      type: "Linear"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.1 }
  }
}
)",
          400,
          0.01,
      },
      {
          "ZeroTotalWeightGuard",
          R"(
width: 32
height: 32
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "Rainbow" }
system {
  min_lifetime: 1
  max_lifetime: 2
  blends {
    enabled: true
    weight: 0.0
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.2 }
  }
  blends {
    enabled: true
    weight: 0.0
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.9 }
  }
}
)",
          1600,
          0.05,
      },
      {
          "IsolatedBlendSecondBranch",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "BGR" }
system {
  min_lifetime: 1
  max_lifetime: 12
  isolated_blend { index: 1 }
  blends {
    enabled: true
    weight: 10.0
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Linear"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.0 }
  }
  blends {
    enabled: true
    weight: 0.2
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.85 }
  }
}
)",
          400,
          0.01,
      },
      {
          "CameraWithAffineFormulaAndColoring",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "Rainbow" }
system {
  min_lifetime: 1
  max_lifetime: 15
  blends {
    enabled: true
    weight: 1.0
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.2 }
  }
  camera {
    enabled: true
    pre { m11: 0.7 m21: -0.3 m31: 0.1 m12: 0.3 m22: 0.7 m32: -0.05 }
    formulas {
      type: "Swirl"
      weight { x: 0.6 }
    }
    formulas {
      type: "Linear"
      weight { x: 0.4 }
    }
    post { m11: 0.85 m22: 0.85 m31: -0.05 m32: 0.05 }
    coloring_method { distance: 0.6 }
  }
}
)",
          400,
          0.01,
      },
      {
          "JuliaDrainCameraAndBlackWhite",
          R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "BlackWhite" }
system {
  min_lifetime: 2
  max_lifetime: 20
  blends {
    enabled: true
    weight: 1.0
    pre { m11: 0.8466 m22: 0.8466 m31: -0.5 m32: 0.25 }
    formulas {
      type: "Julia"
      weight { x: 0.7 }
    }
    formulas {
      type: "Drain"
      params: 0.7
      params: -0.5
      params: 0.3
      params: 0.0
      weight { x: 0.3 }
    }
    coloring_method { single_color: 0.6 }
  }
  camera {
    enabled: true
    pre { m11: 0.9 m22: 0.9 }
    formulas {
      type: "Linear"
      weight { x: 1.0 }
    }
  }
}
)",
          400,
          0.01,
      },
  };

  for (const auto& fixture : cases) {
    const QString projectPath =
        tempDir_.filePath(QString("%1.pbtxt").arg(fixture.name));
    writeTextFile(projectPath, fixture.pbtxt);

    const QString cpuPath =
        tempDir_.filePath(QString("%1_cpu.png").arg(fixture.name));
    const QString gpuPath =
        tempDir_.filePath(QString("%1_gpu.png").arg(fixture.name));

    const QString ippStr = QString::number(fixture.ipp);
    RunResult cpuRes = runChaoskitRender(
        {projectPath, cpuPath, "--backend", "cpu", "--ipp", ippStr});
    ASSERT_EQ(cpuRes.exitCode, 0) << cpuRes.stdErr.toStdString();

    RunResult gpuRes = runChaoskitRender(
        {projectPath, gpuPath, "--backend", "gpu", "--ipp", ippStr});
    ASSERT_EQ(gpuRes.exitCode, 0) << gpuRes.stdErr.toStdString();

    QImage cpuImg(cpuPath);
    QImage gpuImg(gpuPath);
    ASSERT_FALSE(cpuImg.isNull());
    ASSERT_FALSE(gpuImg.isNull());
    EXPECT_EQ(cpuImg.size(), gpuImg.size());

    const LuminanceStats gpuStats = analyzeLuminance(gpuImg);
    EXPECT_TRUE(gpuStats.allOpaque) << fixture.name;
    EXPECT_GT(gpuStats.meanLuminance, fixture.minMeanLuminance) << fixture.name;

    const double rmse = calculateNormalizedRmse(cpuImg, gpuImg, nullptr);
    EXPECT_LE(rmse, 0.05) << "Parity failed for " << fixture.name
                          << " with RMSE=" << rmse;
  }

  // Also check simple.pbtxt and multi_blend_camera.pbtxt at 400 IPP.
  for (const char* fixtureName : {"simple.pbtxt", "multi_blend_camera.pbtxt"}) {
    const QString projectPath = testDataDir_ + "/" + fixtureName;
    const QString cpuPath =
        tempDir_.filePath(QString("cpu_%1.png").arg(fixtureName));
    const QString gpuPath =
        tempDir_.filePath(QString("gpu_%1.png").arg(fixtureName));

    ASSERT_EQ(runChaoskitRender(
                  {projectPath, cpuPath, "--backend", "cpu", "--ipp", "400"})
                  .exitCode,
              0);
    ASSERT_EQ(runChaoskitRender(
                  {projectPath, gpuPath, "--backend", "gpu", "--ipp", "400"})
                  .exitCode,
              0);

    QImage cpuImg(cpuPath);
    QImage gpuImg(gpuPath);
    ASSERT_FALSE(cpuImg.isNull());
    ASSERT_FALSE(gpuImg.isNull());
    EXPECT_LE(calculateNormalizedRmse(cpuImg, gpuImg, nullptr), 0.05)
        << "Parity failed for " << fixtureName;
  }
}

TEST_F(CliTest, GpuSeededDeterminismAndZeroPlottedOpaqueBlack) {
  const QString inputPath = testDataDir_ + "/simple.pbtxt";
  const QString gpu1 = tempDir_.filePath("gpu_seed_1.png");
  const QString gpu2 = tempDir_.filePath("gpu_seed_2.png");
  const QString gpuOtherSeed = tempDir_.filePath("gpu_seed_43.png");

  ASSERT_EQ(
      runChaoskitRender({inputPath, gpu1, "--backend", "gpu", "--seed", "42"})
          .exitCode,
      0);
  ASSERT_EQ(
      runChaoskitRender({inputPath, gpu2, "--backend", "gpu", "--seed", "42"})
          .exitCode,
      0);
  ASSERT_EQ(runChaoskitRender(
                {inputPath, gpuOtherSeed, "--backend", "gpu", "--seed", "43"})
                .exitCode,
            0);

  const QByteArray bytes1 = readFileBytes(gpu1);
  const QByteArray bytes2 = readFileBytes(gpu2);
  const QByteArray bytesOther = readFileBytes(gpuOtherSeed);
  EXPECT_GT(bytes1.size(), 0);
  EXPECT_EQ(bytes1, bytes2);
  EXPECT_NE(bytes1, bytesOther);

  QImage img42(gpu1);
  QImage img43(gpuOtherSeed);
  ASSERT_FALSE(img42.isNull());
  ASSERT_FALSE(img43.isNull());
  EXPECT_GT(calculateNormalizedRmse(img42, img43, nullptr), 0.01);

  // Bitwise determinism on a complex multi-blend + singularity + camera
  // fixture.
  const QString complexPath = tempDir_.filePath("complex_seeded.pbtxt");
  writeTextFile(complexPath, R"(
width: 64
height: 64
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "Rainbow" }
system {
  min_lifetime: 2
  max_lifetime: 15
  blends {
    enabled: true
    weight: 0.6
    formulas {
      type: "DeJong"
      params: -1.9292302
      params: -1.7559409
      params: -1.8413773
      params: -1.9726439
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.3 }
  }
  blends {
    enabled: true
    weight: 0.2
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.7 }
  }
  blends {
    enabled: true
    weight: 0.2
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Hyperbolic"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.9 }
  }
  camera {
    enabled: true
    pre { m11: 0.8 m21: -0.2 m31: 0.05 m12: 0.2 m22: 0.8 m32: -0.05 }
    formulas {
      type: "Swirl"
      weight { x: 0.6 }
    }
    formulas {
      type: "Linear"
      weight { x: 0.4 }
    }
    post { m11: 0.9 m22: 0.9 }
    coloring_method { distance: 0.5 }
  }
}
)");

  const QString complexGpu1 = tempDir_.filePath("complex_gpu_1.png");
  const QString complexGpu2 = tempDir_.filePath("complex_gpu_2.png");
  const QString complexAuto = tempDir_.filePath("complex_auto.png");
  ASSERT_EQ(runChaoskitRender({complexPath, complexGpu1, "--backend", "gpu",
                               "--seed", "123456789"})
                .exitCode,
            0);
  ASSERT_EQ(runChaoskitRender({complexPath, complexGpu2, "--backend", "gpu",
                               "--seed", "123456789"})
                .exitCode,
            0);
  ASSERT_EQ(runChaoskitRender({complexPath, complexAuto, "--seed", "123456789"})
                .exitCode,
            0);

  const QByteArray complexBytes1 = readFileBytes(complexGpu1);
  EXPECT_GT(complexBytes1.size(), 0);
  EXPECT_EQ(complexBytes1, readFileBytes(complexGpu2));
  EXPECT_EQ(complexBytes1, readFileBytes(complexAuto));

  // Verify degenerate zero-plotted project renders opaque black on GPU.
  const QString zeroIppPath = tempDir_.filePath("gpu_zero_ipp.pbtxt");
  writeTextFile(zeroIppPath, R"(
width: 32
height: 32
gamma: 2.2
exposure: 0.0
vibrancy: 0.0
color_map { name: "BGR" }
system {
  min_lifetime: 0
  max_lifetime: 1
  blends {
    enabled: true
    weight: 1.0
    pre { m11: 0.0 m22: 0.0 }
    formulas {
      type: "Spherical"
      weight { x: 1.0 }
    }
    coloring_method { single_color: 0.5 }
  }
}
)");

  const QString zeroOut = tempDir_.filePath("gpu_zero_ipp.png");
  ASSERT_EQ(runChaoskitRender({zeroIppPath, zeroOut, "--backend", "gpu",
                               "--iterations", "10"})
                .exitCode,
            0);
  QImage zeroImg(zeroOut);
  ASSERT_FALSE(zeroImg.isNull());
  const LuminanceStats stats = analyzeLuminance(zeroImg);
  EXPECT_TRUE(stats.allOpaque);
  EXPECT_EQ(stats.opaqueBlackPixels, 32u * 32u);
}

TEST_F(CliTest, ExplicitBackendAutoAndCpuSelection) {
  const QString inputPath = testDataDir_ + "/simple.pbtxt";
  const QString autoOut = tempDir_.filePath("auto.png");
  const QString cpuOut = tempDir_.filePath("cpu.png");

  RunResult autoRes = runChaoskitRender(
      {inputPath, autoOut, "--backend", "auto", "--ipp", "200"});
  ASSERT_EQ(autoRes.exitCode, 0) << autoRes.stdErr.toStdString();
  QImage autoImg(autoOut);
  ASSERT_FALSE(autoImg.isNull());
  EXPECT_EQ(autoImg.size(), QSize(64, 64));

  RunResult cpuRes = runChaoskitRender(
      {inputPath, cpuOut, "--backend", "cpu", "--seed", "42"});
  ASSERT_EQ(cpuRes.exitCode, 0) << cpuRes.stdErr.toStdString();
  QImage cpuImg(cpuOut);
  QImage goldenImg(testDataDir_ + "/simple_golden.png");
  ASSERT_FALSE(cpuImg.isNull());
  ASSERT_FALSE(goldenImg.isNull());
  EXPECT_LE(calculateNormalizedRmse(cpuImg, goldenImg, nullptr), 0.05);
}

TEST_F(CliTest, AutoBackendWithSeedMatchesGpuByteForByte) {
  const QString inputPath = testDataDir_ + "/simple.pbtxt";
  const QString defaultOut = tempDir_.filePath("default_seed.png");
  const QString autoOut = tempDir_.filePath("auto_seed.png");
  const QString gpuOut = tempDir_.filePath("gpu_seed.png");
  const QString cpuOut = tempDir_.filePath("cpu_seed.png");

  ASSERT_EQ(runChaoskitRender({inputPath, defaultOut, "--seed", "42"}).exitCode,
            0);
  ASSERT_EQ(runChaoskitRender(
                {inputPath, autoOut, "--backend", "auto", "--seed", "42"})
                .exitCode,
            0);
  ASSERT_EQ(
      runChaoskitRender({inputPath, gpuOut, "--backend", "gpu", "--seed", "42"})
          .exitCode,
      0);
  ASSERT_EQ(
      runChaoskitRender({inputPath, cpuOut, "--backend", "cpu", "--seed", "42"})
          .exitCode,
      0);

  const QByteArray defaultBytes = readFileBytes(defaultOut);
  const QByteArray autoBytes = readFileBytes(autoOut);
  const QByteArray gpuBytes = readFileBytes(gpuOut);
  const QByteArray cpuBytes = readFileBytes(cpuOut);

  EXPECT_GT(defaultBytes.size(), 0);
  EXPECT_EQ(defaultBytes, autoBytes);
  EXPECT_EQ(defaultBytes, gpuBytes);
  EXPECT_NE(defaultBytes, cpuBytes);
}

}  // namespace
}  // namespace chaoskit::cli
