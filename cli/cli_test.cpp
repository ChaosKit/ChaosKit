#include <gmock/gmock.h>
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
#include <cstdlib>

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

class CliTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
#if !defined(Q_OS_MACOS)
    qputenv("QT_QPA_PLATFORM", "offscreen");
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
  QString actualPath = tempDir_.filePath("simple_actual.png");
  QString goldenPath = testDataDir_ + "/simple_golden.png";

  RunResult res = runChaoskitRender({inputPath, actualPath, "--seed", "42"});
  EXPECT_EQ(res.exitCode, 0);

  bool updateGoldens = qEnvironmentVariableIsSet("CHAOSKIT_UPDATE_GOLDENS") &&
                       qEnvironmentVariable("CHAOSKIT_UPDATE_GOLDENS") != "0";
  if (updateGoldens) {
    if (QFile::exists(goldenPath)) {
      QFile::remove(goldenPath);
    }
    ASSERT_TRUE(QFile::copy(actualPath, goldenPath));
  }

  QImage actualImage(actualPath);
  ASSERT_FALSE(actualImage.isNull())
      << "Failed to load actual image: " << actualPath.toStdString();

  QImage goldenImage(goldenPath);
  if (goldenImage.isNull()) {
    actualImage.save("simple_actual.png");
    FAIL() << "Golden image missing or invalid at " << goldenPath.toStdString()
           << ". Set CHAOSKIT_UPDATE_GOLDENS=1 to generate.";
  }

  QImage diffImage;
  double rmse = calculateNormalizedRmse(actualImage, goldenImage, &diffImage);
  if (rmse > 0.05) {
    actualImage.save("simple_actual.png");
    diffImage.save("simple_diff.png");
  }

  EXPECT_LE(rmse, 0.05);
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
  EXPECT_LE(rmse, 0.05);
}

}  // namespace
}  // namespace chaoskit::cli
