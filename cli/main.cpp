#include <google/protobuf/text_format.h>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QImageWriter>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
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
#include "core/CameraSystemProcessor.h"
#include "core/ColorMapRegistry.h"
#include "core/Renderer.h"
#include "core/ThreadLocalRng.h"
#include "core/errors.h"
#include "ui/GLToneMapper.h"
#include "ui/GathererTask.h"
#include "ui/storage/conversions.h"

using namespace chaoskit;

namespace {

class BufferExtractAdapter : public core::Renderer {
 public:
  void updateHistogramBuffer(const core::HistogramBuffer& buffer) override {
    buffer_ = buffer;
  }

  core::HistogramBuffer buffer_;
};

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

void stepParticles(uint64_t budget, core::CameraSystemProcessor& processor,
                   const core::ColorMap* colorMap, uint32_t width,
                   uint32_t height, core::HistogramBuffer& bufferOut) {
  ui::GathererTask gatherer;
  gatherer.setSize(QSize(static_cast<int>(width), static_cast<int>(height)));
  gatherer.setColorMap(colorMap);

  auto particle = processor.createParticle();
  for (uint64_t step = 0; step < budget; ++step) {
    particle = processor.process(particle);
    auto output = processor.processCamera(particle);
    gatherer.addPoint(output.point, output.color);
  }

  BufferExtractAdapter adapter;
  gatherer.synchronizeResult(&adapter);
  bufferOut = std::move(adapter.buffer_);
}

}  // namespace

int main(int argc, char* argv[]) {
#if defined(Q_OS_MACOS)
  // On macOS, Qt's offscreen QPA platform plugin does not support OpenGL
  // context creation. Cocoa platform supports offscreen rendering via
  // QOffscreenSurface without showing a window.
  qunsetenv("QT_QPA_PLATFORM");
#else
  qputenv("QT_QPA_PLATFORM", "offscreen");
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

  // Particle stepping & gathering
  core::HistogramBuffer combinedBuffer(width, height);
  combinedBuffer.clear();

  if (parser.isSet(seedOption)) {
    core::CameraSystemProcessor processor(cameraSystem, rng);
    stepParticles(totalBudget, processor, colorMap, width, height,
                  combinedBuffer);
  } else if (threads == 1) {
    core::CameraSystemProcessor processor(
        cameraSystem, std::make_shared<core::ThreadLocalRng>());
    stepParticles(totalBudget, processor, colorMap, width, height,
                  combinedBuffer);
  } else {
    size_t threadCount = threads;
    std::vector<core::HistogramBuffer> threadBuffers(threadCount);
    std::vector<std::thread> workerThreads;
    workerThreads.reserve(threadCount);

    uint64_t share = totalBudget / threadCount;
    uint64_t remainder = totalBudget % threadCount;

    for (size_t i = 0; i < threadCount; ++i) {
      uint64_t threadBudget = share + (i < remainder ? 1 : 0);
      workerThreads.emplace_back([&, i, threadBudget]() {
        core::CameraSystemProcessor processor(
            cameraSystem, std::make_shared<core::ThreadLocalRng>());
        stepParticles(threadBudget, processor, colorMap, width, height,
                      threadBuffers[i]);
      });
    }

    for (size_t i = 0; i < workerThreads.size(); ++i) {
      workerThreads[i].join();
    }

    auto* dst = combinedBuffer.data();
    for (const auto& tb : threadBuffers) {
      const auto* src = tb.data();
      for (size_t idx = 0; idx < combinedBuffer.size(); ++idx) {
        dst[idx] += src[idx];
      }
    }
  }

  // OpenGL offscreen setup
  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  QOpenGLContext context;
  context.setFormat(format);
  if (!context.create()) {
    std::cerr << "Failed to create OpenGL context" << std::endl;
    return 1;
  }

  QOffscreenSurface surface;
  surface.setFormat(format);
  surface.create();
  if (!surface.isValid()) {
    std::cerr << "Failed to create offscreen surface" << std::endl;
    return 1;
  }

  if (!context.makeCurrent(&surface)) {
    std::cerr << "Failed to make OpenGL context current" << std::endl;
    return 1;
  }

  QOpenGLFramebufferObject fbo(static_cast<int>(width),
                               static_cast<int>(height));
  if (!fbo.isValid() || !fbo.bind()) {
    std::cerr << "Failed to bind framebuffer object" << std::endl;
    return 1;
  }

  auto* gl = context.functions();
  gl->glViewport(0, 0, static_cast<GLsizei>(width),
                 static_cast<GLsizei>(height));

  ui::GLToneMapper toneMapper;
  toneMapper.initializeGL();
  toneMapper.setGamma(gamma);
  toneMapper.setExposure(exposure);
  toneMapper.setVibrancy(vibrancy);

  toneMapper.syncBuffer(combinedBuffer);

  toneMapper.map();

  QImage image = fbo.toImage();
  fbo.release();

  QImageWriter writer(outputPath);
  if (!writer.write(image)) {
    std::cerr << "Failed to write image: " << writer.errorString().toStdString()
              << '\n';
    return 1;
  }

  return 0;
}
