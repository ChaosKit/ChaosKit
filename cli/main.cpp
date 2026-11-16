#include <google/protobuf/text_format.h>
#include <rhi/qrhi.h>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QImageWriter>
#include <QSize>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "SeededRng.h"
#include "chaoskit.pb.h"
#include "core/ColorMap.h"
#include "core/ColorMapRegistry.h"
#include "core/CpuAccumulator.h"
#include "core/Rng.h"
#include "core/ThreadLocalRng.h"
#include "core/errors.h"
#include "ui/RhiOffscreenContext.h"
#include "ui/RhiToneMapper.h"
#include "ui/storage/conversions.h"

using namespace chaoskit;

namespace {

bool parsePositiveInt64(const QString& str, int64_t* out) {
  QString trimmed = str.trimmed();
  if (trimmed.isEmpty()) {
    return false;
  }
  for (int i = 0; i < trimmed.size(); ++i) {
    if (!trimmed.at(i).isDigit()) {
      return false;
    }
  }
  bool ok = false;
  qlonglong val = trimmed.toLongLong(&ok);
  if (!ok || val <= 0) {
    return false;
  }
  if (out != nullptr) {
    *out = val;
  }
  return true;
}

bool parseFloat(const QString& value, const QString& flagName, float& out,
                bool mustBePositive = false) {
  bool ok = false;
  float val = value.toFloat(&ok);
  if (!ok || !std::isfinite(val) || (mustBePositive && val <= 0.0f)) {
    std::cerr << "Invalid " << flagName.toStdString() << ": "
              << value.toStdString() << std::endl;
    return false;
  }
  out = val;
  return true;
}

}  // namespace

int main(int argc, char* argv[]) {
#if defined(Q_OS_MACOS)
  // Prefer Cocoa over Qt's minimal offscreen QPA plugin on macOS while keeping
  // rendering windowless via swapchain-free QRhi.
  qunsetenv("QT_QPA_PLATFORM");
#else
  if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM") &&
      !qEnvironmentVariableIsSet("DISPLAY") &&
      !qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
  }
#endif
  QGuiApplication app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("ChaosKit headless renderer");
  parser.addHelpOption();
  parser.addPositionalArgument("input", "Input project file (.ck or .pbtxt)");
  parser.addPositionalArgument("output", "Output image file");

  QCommandLineOption seedOption(
      "seed", "Random number generator seed (64-bit integer)", "uint64");
  parser.addOption(seedOption);

  QCommandLineOption threadsOption(
      "threads", "Number of worker threads (default: hardware concurrency)",
      "count");
  parser.addOption(threadsOption);

  QCommandLineOption ippOption({"ipp", "iterations-per-pixel"},
                               "Iterations per pixel (default: 100)", "count");
  parser.addOption(ippOption);

  QCommandLineOption iterationsOption(
      "iterations", "Exact total plotted step budget", "count");
  parser.addOption(iterationsOption);

  QCommandLineOption widthOption(
      "width", "Override image width in pixels (must be > 0)", "uint");
  parser.addOption(widthOption);

  QCommandLineOption heightOption(
      "height", "Override image height in pixels (must be > 0)", "uint");
  parser.addOption(heightOption);

  QCommandLineOption gammaOption(
      "gamma", "Override gamma correction factor (must be > 0)", "float");
  parser.addOption(gammaOption);

  QCommandLineOption exposureOption("exposure", "Override exposure adjustment",
                                    "float");
  parser.addOption(exposureOption);

  QCommandLineOption vibrancyOption(
      "vibrancy", "Override color vibrancy adjustment", "float");
  parser.addOption(vibrancyOption);

  QCommandLineOption colorMapOption("color-map", "Override color map by name",
                                    "name");
  parser.addOption(colorMapOption);

  parser.process(app);

  const QStringList positionalArgs = parser.positionalArguments();
  if (positionalArgs.size() != 2) {
    std::cerr << "Usage: chaoskit-render <input-project> <output-image> "
                 "[--seed <uint64>] [--threads <count>] "
                 "[--ipp <count>] [--iterations <count>] "
                 "[--width <uint>] [--height <uint>] "
                 "[--gamma <float>] [--exposure <float>] [--vibrancy <float>] "
                 "[--color-map <name>]"
              << std::endl;
    return 1;
  }

  if (parser.isSet(ippOption) && parser.isSet(iterationsOption)) {
    std::cerr
        << "Error: cannot specify both --ipp (--iterations-per-pixel) and "
           "--iterations"
        << std::endl;
    return 1;
  }

  size_t threads = (std::max)(1u, std::thread::hardware_concurrency());
  if (parser.isSet(threadsOption)) {
    int64_t val = 0;
    if (!parsePositiveInt64(parser.value(threadsOption), &val)) {
      std::cerr << "Invalid --threads: "
                << parser.value(threadsOption).toStdString() << std::endl;
      return 1;
    }
    threads = static_cast<size_t>(val);
  }

  if (parser.isSet(seedOption) && parser.isSet(threadsOption) && threads > 1) {
    std::cerr
        << "Error: cannot specify --threads > 1 with --seed (deterministic "
           "mode is single-threaded)"
        << std::endl;
    return 1;
  }

  uint64_t ipp = 100;
  if (parser.isSet(ippOption)) {
    int64_t val = 0;
    if (!parsePositiveInt64(parser.value(ippOption), &val)) {
      std::cerr << "Invalid --ipp / --iterations-per-pixel: "
                << parser.value(ippOption).toStdString() << std::endl;
      return 1;
    }
    ipp = static_cast<uint64_t>(val);
  }

  std::optional<uint64_t> exactIterations = std::nullopt;
  if (parser.isSet(iterationsOption)) {
    int64_t val = 0;
    if (!parsePositiveInt64(parser.value(iterationsOption), &val)) {
      std::cerr << "Invalid --iterations: "
                << parser.value(iterationsOption).toStdString() << std::endl;
      return 1;
    }
    exactIterations = static_cast<uint64_t>(val);
  }

  const QString inputPath = positionalArgs.at(0);
  const QString outputPath = positionalArgs.at(1);

  // Read input file
  QFile inputFile(inputPath);
  if (!inputFile.open(QIODevice::ReadOnly)) {
    std::cerr << "Failed to open input file " << inputPath.toStdString() << ": "
              << inputFile.errorString().toStdString() << std::endl;
    return 1;
  }
  QByteArray fileData = inputFile.readAll();
  inputFile.close();

  // Parse project (binary .ck or text protobuf)
  chaoskit::Project project;
  std::string dataStr(fileData.constData(), fileData.size());
  bool isTextFormat = inputPath.endsWith(".pbtxt", Qt::CaseInsensitive) ||
                      inputPath.endsWith(".cktxt", Qt::CaseInsensitive);
  bool parsed = false;
  if (isTextFormat) {
    parsed = google::protobuf::TextFormat::ParseFromString(dataStr, &project);
    if (!parsed) {
      parsed = project.ParseFromString(dataStr);
    }
  } else {
    parsed = project.ParseFromString(dataStr);
    if (!parsed) {
      parsed = google::protobuf::TextFormat::ParseFromString(dataStr, &project);
    }
  }
  if (!parsed) {
    std::cerr << "Failed to parse project file: " << inputPath.toStdString()
              << std::endl;
    return 1;
  }

  // Default normalization
  uint32_t width = project.width() == 0 ? 1024 : project.width();
  uint32_t height = project.height() == 0 ? 1024 : project.height();
  float gamma = project.gamma() == 0.0f ? 2.2f : project.gamma();
  float exposure = project.exposure();
  float vibrancy = project.vibrancy();

  // CLI overrides
  if (parser.isSet(widthOption)) {
    bool ok = false;
    uint32_t val = parser.value(widthOption).toUInt(&ok);
    if (!ok || val == 0) {
      std::cerr << "Invalid width: " << parser.value(widthOption).toStdString()
                << std::endl;
      return 1;
    }
    width = val;
  }

  if (parser.isSet(heightOption)) {
    bool ok = false;
    uint32_t val = parser.value(heightOption).toUInt(&ok);
    if (!ok || val == 0) {
      std::cerr << "Invalid height: "
                << parser.value(heightOption).toStdString() << std::endl;
      return 1;
    }
    height = val;
  }

  if (parser.isSet(gammaOption)) {
    if (!parseFloat(parser.value(gammaOption), "gamma", gamma,
                    /*mustBePositive=*/true)) {
      return 1;
    }
  }

  if (parser.isSet(exposureOption)) {
    if (!parseFloat(parser.value(exposureOption), "exposure", exposure)) {
      return 1;
    }
  }

  if (parser.isSet(vibrancyOption)) {
    if (!parseFloat(parser.value(vibrancyOption), "vibrancy", vibrancy)) {
      return 1;
    }
  }

  core::ColorMapRegistry colorMapRegistry;
  std::string colorMapName = project.color_map().name();
  if (parser.isSet(colorMapOption)) {
    colorMapName = parser.value(colorMapOption).toStdString();
  }
  if (colorMapName.empty()) {
    colorMapName = colorMapRegistry.defaultName();
  }

  const core::ColorMap* colorMap = nullptr;
  try {
    colorMap = colorMapRegistry.get(colorMapName);
  } catch (const core::InvalidColorMap&) {
    std::cerr << "Unrecognized color map: " << colorMapName << std::endl;
    return 1;
  }

  // Lowering
  std::vector<std::string> warnings;
  auto cameraSystem = ui::toCameraSystem(project.system(), &warnings);
  for (const auto& warning : warnings) {
    std::cerr << "Warning: " << warning << std::endl;
  }
  if (project.system().has_isolated_blend()) {
    std::cerr << "Warning: system has isolated blend (index "
              << project.system().isolated_blend().index() << ")" << std::endl;
  }

  // RNG selection
  std::shared_ptr<core::Rng> rng;
  if (parser.isSet(seedOption)) {
    bool ok = false;
    uint64_t seed = parser.value(seedOption).toULongLong(&ok);
    if (!ok) {
      std::cerr << "Invalid seed: " << parser.value(seedOption).toStdString()
                << std::endl;
      return 1;
    }
    rng = std::make_shared<cli::SeededRng>(seed);
  }

  uint64_t totalBudget = 0;
  if (exactIterations.has_value()) {
    totalBudget = *exactIterations;
  } else {
    totalBudget = ipp * static_cast<uint64_t>(width) * height;
  }

  // Particle stepping & gathering via shared Qt-free CpuAccumulator
  std::shared_ptr<core::Rng> singleRng =
      rng ? rng : std::make_shared<core::ThreadLocalRng>();
  core::CpuAccumulator accumulator(cameraSystem, width, height, colorMap,
                                   singleRng);

  if (parser.isSet(seedOption) || threads == 1) {
    accumulator.accumulatePlotted(totalBudget);
  } else {
    const size_t threadCount = threads;
    std::vector<std::unique_ptr<core::CpuAccumulator>> workerAccumulators;
    workerAccumulators.reserve(threadCount);
    for (size_t i = 0; i < threadCount; ++i) {
      workerAccumulators.push_back(std::make_unique<core::CpuAccumulator>(
          cameraSystem, width, height, colorMap,
          std::make_shared<core::ThreadLocalRng>()));
    }

    std::vector<std::thread> workerThreads;
    workerThreads.reserve(threadCount);

    const uint64_t share = totalBudget / threadCount;
    const uint64_t remainder = totalBudget % threadCount;

    for (size_t i = 0; i < threadCount; ++i) {
      const uint64_t threadBudget = share + (i < remainder ? 1 : 0);
      auto* workerAcc = workerAccumulators[i].get();
      workerThreads.emplace_back([workerAcc, threadBudget]() {
        workerAcc->accumulatePlotted(threadBudget);
      });
    }

    for (size_t i = 0; i < workerThreads.size(); ++i) {
      workerThreads[i].join();
    }

    for (const auto& workerAcc : workerAccumulators) {
      accumulator.merge(*workerAcc);
    }
  }

  // Swapchain-free offscreen QRhi tone mapping & readback
  QString rhiError;
  auto rhiContext = ui::RhiOffscreenContext::create(&rhiError);
  if (!rhiContext) {
    std::cerr << "Failed to initialize offscreen QRhi context: "
              << rhiError.toStdString() << std::endl;
    return 1;
  }
  QRhi* rhi = rhiContext->rhi();

  const QSize outputSize(static_cast<int>(width), static_cast<int>(height));
  std::unique_ptr<QRhiTexture> colorTexture(rhi->newTexture(
      QRhiTexture::RGBA8, outputSize, 1,
      QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
  if (!colorTexture || !colorTexture->create()) {
    std::cerr << "Failed to create offscreen output texture" << std::endl;
    return 1;
  }

  std::unique_ptr<QRhiTextureRenderTarget> renderTarget(
      rhi->newTextureRenderTarget(
          QRhiTextureRenderTargetDescription(colorTexture.get())));
  std::unique_ptr<QRhiRenderPassDescriptor> renderPassDesc(
      renderTarget->newCompatibleRenderPassDescriptor());
  renderTarget->setRenderPassDescriptor(renderPassDesc.get());
  if (!renderTarget->create()) {
    std::cerr << "Failed to create offscreen render target" << std::endl;
    return 1;
  }

  ui::RhiToneMapper toneMapper;
  if (!toneMapper.initialize(rhi, renderPassDesc.get(), 1, &rhiError)) {
    std::cerr << "Failed to initialize RhiToneMapper: "
              << rhiError.toStdString() << std::endl;
    return 1;
  }
  toneMapper.setGamma(gamma);
  toneMapper.setExposure(exposure);
  toneMapper.setVibrancy(vibrancy);

  QRhiCommandBuffer* cb = nullptr;
  if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess) {
    std::cerr << "Failed to begin offscreen QRhi frame" << std::endl;
    return 1;
  }

  QRhiReadbackResult readbackResult;
  QRhiResourceUpdateBatch* readbackBatch = rhi->nextResourceUpdateBatch();
  readbackBatch->readBackTexture(QRhiReadbackDescription(colorTexture.get()),
                                 &readbackResult);

  toneMapper.render(cb, renderTarget.get(), &accumulator.histogram(),
                    accumulator.plottedIterations(), readbackBatch);

  if (rhi->endOffscreenFrame() != QRhi::FrameOpSuccess) {
    std::cerr << "Failed to end offscreen QRhi frame" << std::endl;
    return 1;
  }

  QImage image = ui::imageFromRhiReadback(rhi, readbackResult);
  if (image.isNull()) {
    std::cerr << "Failed to convert RHI readback to QImage" << std::endl;
    return 1;
  }

  QImageWriter writer(outputPath);
  if (!writer.write(image)) {
    std::cerr << "Failed to write image: " << writer.errorString().toStdString()
              << '\n';
    return 1;
  }

  return 0;
}
