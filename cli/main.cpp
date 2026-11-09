#include <google/protobuf/text_format.h>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <iostream>
#include <memory>
#include <string>
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

class ToneMapperAdapter : public core::Renderer {
 public:
  explicit ToneMapperAdapter(ui::GLToneMapper* toneMapper)
      : toneMapper_(toneMapper) {}

  void updateHistogramBuffer(const core::HistogramBuffer& buffer) override {
    toneMapper_->syncBuffer(buffer);
  }

 private:
  ui::GLToneMapper* toneMapper_;
};

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

  parser.process(app);

  const QStringList positionalArgs = parser.positionalArguments();
  if (positionalArgs.size() != 2) {
    std::cerr << "Usage: chaoskit-render <input-project> <output-image> "
                 "[--seed <uint64>]"
              << std::endl;
    return 1;
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
  bool parsed = project.ParseFromString(dataStr);
  if (!parsed) {
    parsed = google::protobuf::TextFormat::ParseFromString(dataStr, &project);
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

  core::ColorMapRegistry colorMapRegistry;
  std::string colorMapName = project.color_map().name();
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
  } else {
    rng = std::make_shared<core::ThreadLocalRng>();
  }

  // Particle stepping
  core::CameraSystemProcessor processor(cameraSystem, rng);
  ui::GathererTask gatherer;
  gatherer.setSize(QSize(static_cast<int>(width), static_cast<int>(height)));
  gatherer.setColorMap(colorMap);

  size_t budget = static_cast<size_t>(100) * width * height;
  auto particle = processor.createParticle();
  for (size_t i = 0; i < budget; ++i) {
    particle = processor.process(particle);
    auto output = processor.processCamera(particle);
    gatherer.addPoint(output.point, output.color);
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

  ToneMapperAdapter adapter(&toneMapper);
  gatherer.synchronizeResult(&adapter);

  toneMapper.map();

  QImage image = fbo.toImage();
  fbo.release();

  if (!image.save(outputPath)) {
    std::cerr << "Failed to save image to " << outputPath.toStdString()
              << std::endl;
    return 1;
  }

  return 0;
}
