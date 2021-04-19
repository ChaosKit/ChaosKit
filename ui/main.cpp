#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QImageWriter>
#include <QItemSelectionModel>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QRegularExpression>
#include <QSurfaceFormat>
#include <QtGui/QTransform>
#include <magic_enum.hpp>
#include "ColorMap.h"
#include "ColorMapPreviewProvider.h"
#include "ColorMapRegistry.h"
#include "DocumentModel.h"
#include "DocumentProxy.h"
#include "EngineManager.h"
#include "FormulaPreviewProvider.h"
#include "HistogramBuffer.h"
#include "ModelEntry.h"
#include "Point.h"
#include "SystemView.h"
#include "Utilities.h"
#include "core/PaletteColorMap.h"
#include "library/FormulaType.h"
#include "resources.h"

using chaoskit::core::Color;
using chaoskit::core::ColorMap;
using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::library::FormulaType;
using chaoskit::ui::ColorMapPreviewProvider;
using chaoskit::ui::ColorMapRegistry;
using chaoskit::ui::DocumentEntryType;
using chaoskit::ui::DocumentModel;
using chaoskit::ui::DocumentProxy;
using chaoskit::ui::EngineManager;
using chaoskit::ui::FormulaPreviewProvider;
using chaoskit::ui::ModelEntry;
using chaoskit::ui::SystemView;
using chaoskit::ui::Utilities;
namespace resources = chaoskit::resources;

QStringList createFormulaList() {
  QStringList result;
  for (const auto& name : magic_enum::enum_names<FormulaType>()) {
    QString formula = QString::fromUtf8(name.data(), name.size());
    if (formula != QStringLiteral("Invalid")) {
      result.append(formula);
    }
  }
  return result;
}

QStringList createExportFormatList() {
  static const QSet<QByteArray> excludedFormats{
      QByteArrayLiteral("cur"), QByteArrayLiteral("icns"),
      QByteArrayLiteral("ico"), QByteArrayLiteral("wbmp")};
  static const QHash<QByteArray, QString> formatLabels{
      {QByteArrayLiteral("bmp"), QStringLiteral("Windows Bitmap (*.bmp)")},
      {QByteArrayLiteral("heic"),
       QStringLiteral("High Efficiency Image Format (*.heic *.heif)")},
      {QByteArrayLiteral("heif"),
       QStringLiteral("High Efficiency Image Format (*.heic *.heif)")},
      {QByteArrayLiteral("jp2"), QStringLiteral("JPEG 2000 (*.jp2)")},
      {QByteArrayLiteral("jpeg"), QStringLiteral("JPEG (*.jpeg *.jpg)")},
      {QByteArrayLiteral("jpg"), QStringLiteral("JPEG (*.jpeg *.jpg)")},
      {QByteArrayLiteral("pbm"), QStringLiteral("Portable Bitmap (*.pbm)")},
      {QByteArrayLiteral("pgm"), QStringLiteral("Portable Graymap (*.pgm)")},
      {QByteArrayLiteral("png"), QStringLiteral("PNG (*.png)")},
      {QByteArrayLiteral("ppm"), QStringLiteral("Portable Pixmap (*.ppm)")},
      {QByteArrayLiteral("tif"), QStringLiteral("TIFF (*.tif *.tiff)")},
      {QByteArrayLiteral("tiff"), QStringLiteral("TIFF (*.tif *.tiff)")},
      {QByteArrayLiteral("webp"), QStringLiteral("WebP (*.webp)")},
      {QByteArrayLiteral("xbm"), QStringLiteral("X Bitmap (*.xbm)")},
      {QByteArrayLiteral("xpm"), QStringLiteral("X Pixmap (*.xpm)")},
  };

  QStringList result;
  for (const QByteArray& format : QImageWriter::supportedImageFormats()) {
    if (excludedFormats.contains(format)) {
      continue;
    }

    if (formatLabels.contains(format)) {
      result.append(formatLabels.value(format));
    } else {
      auto formatString = QString::fromLatin1(format);
      result.append(QStringLiteral("%1 file (*.%2)")
                        .arg(formatString.toUpper(), formatString));
    }
  }

  result.removeDuplicates();
  result.append(QStringLiteral("All files (*)"));
  return result;
}

int main(int argc, char* argv[]) {
  resources::initialize();

  QQuickStyle::addStylePath(resources::importPath());

  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  // Register types
  qRegisterMetaType<ColorMap*>();
  qRegisterMetaType<HistogramBuffer>();
  qRegisterMetaType<Point>();

  // Set up color maps
  auto* colorMapRegistry = new ColorMapRegistry(&app);
  colorMapRegistry->add(
      "BGR",
      std::make_unique<chaoskit::core::PaletteColorMap>(std::vector<Color>{
          Color{0.f, 0.f, 1.f},
          Color{0.f, 1.f, 0.f},
          Color{1.f, 0.f, 0.f},
      }));

  // Set up models

  auto* documentModel = new DocumentModel();
  documentModel->setColorMapRegistry(colorMapRegistry);
  documentModel->setItemData(documentModel->documentIndex(),
                             {{DocumentModel::ColorMapRole, "BGR"},
                              {DocumentModel::WidthRole, 1024},
                              {DocumentModel::HeightRole, 1024}});
  QModelIndex blendIndex = documentModel->addBlend(FormulaType::DeJong);
  documentModel->setData(blendIndex, QVariant("Distance"),
                         DocumentModel::ColoringMethodTypeRole);
  documentModel->setData(blendIndex, .2f,
                         DocumentModel::ColoringMethodParamRole);
  QModelIndex formulaIndex = documentModel->formulaAt(0, blendIndex);
  documentModel->setData(formulaIndex,
                         QVariant::fromValue(std::vector<float>{
                             9.379666578024626e-01f, 1.938709271140397e+00f,
                             -1.580897020176053e-01f, -1.430070123635232e+00f}),
                         DocumentModel::ParamsRole);
  documentModel->setData(
      documentModel->finalBlendIndex(),
      QVariant::fromValue(QTransform::fromScale(.5, 1).translate(.5, .5)),
      DocumentModel::PostTransformRole);

  auto* selectionModel = new QItemSelectionModel(documentModel);

  // Set up fonts
  {
    QDirIterator it(":/fonts", {"*.otf", "*.ttf"}, QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QFontDatabase::addApplicationFont(it.next());
    }
  }

  // Set up QML and pass data to the context
  auto* engineManager = new EngineManager();
  engineManager->setLoadUrl(resources::createUrl("forms/MainWindow.qml"));

  QObject::connect(engineManager, &EngineManager::engineAboutToBeCreated, [] {
    QQuickStyle::setStyle("ChaosKit");

    qmlRegisterAnonymousType<DocumentProxy>("ChaosKit", 1);
    qmlRegisterUncreatableType<DocumentEntryType>(
        "ChaosKit", 1, 0, "DocumentEntryType",
        "Not creatable because it's an enum");
    // TODO: figure out a non-deprecated way to make ModelEntry work
    qmlRegisterInterface<ModelEntry>("ModelEntry");
    qmlRegisterType<SystemView>("ChaosKit", 1, 0, "SystemView");
    qmlRegisterSingletonType<Utilities>(
        "ChaosKit", 1, 0, "Utilities",
        [](QQmlEngine*, QJSEngine*) -> QObject* { return new Utilities(); });
  });

  auto onEngineCreated = [documentModel, selectionModel, engineManager,
                          colorMapRegistry](QQmlApplicationEngine* engine) {
    engine->addImportPath(resources::importPath());
    engine->addImageProvider("formula", new FormulaPreviewProvider);
    engine->addImageProvider("colormap",
                             new ColorMapPreviewProvider(colorMapRegistry));

    auto exportFormats = createExportFormatList();
    QRegularExpression pngRx(".*\\.png.*");
    int defaultExportFormat = exportFormats.indexOf(pngRx);
    if (defaultExportFormat < 0) {
      defaultExportFormat = exportFormats.size() - 1;
    }

    QQmlContext* rootContext = engine->rootContext();
    rootContext->setContextProperty("defaultExportFormat", defaultExportFormat);
    rootContext->setContextProperty("documentModel", documentModel);
    rootContext->setContextProperty("exportFormats", exportFormats);
    rootContext->setContextProperty("formulaList", createFormulaList());
    rootContext->setContextProperty("globalColorMapRegistry", colorMapRegistry);
    rootContext->setContextProperty(
        "monospaceFont", QFontDatabase::systemFont(QFontDatabase::FixedFont));
    rootContext->setContextProperty("selectionModel", selectionModel);
  };
  QObject::connect(engineManager, &EngineManager::engineCreated,
                   onEngineCreated);

  // Hot reload
  if (!resources::areStatic()) {
    engineManager->enableHotReload();
    engineManager->addWatchDirectory(resources::importPath());
    qInfo() << "Hot Reloader enabled";
  }

  engineManager->load();

  return QGuiApplication::exec();
}
