#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QtGui/QTransform>
#include "DocumentModel.h"
#include "EngineManager.h"
#include "FormulaPreviewProvider.h"
#include "HistogramBuffer.h"
#include "ModelEntry.h"
#include "Point.h"
#include "SystemView.h"
#include "library/FormulaType.h"
#include "resources.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::library::FormulaType;
using chaoskit::ui::DocumentEntryType;
using chaoskit::ui::DocumentModel;
using chaoskit::ui::EngineManager;
using chaoskit::ui::FormulaPreviewProvider;
using chaoskit::ui::ModelEntry;
using chaoskit::ui::SystemView;
namespace resources = chaoskit::resources;

QStringList createFormulaList() {
  QStringList result;
  for (const char* name : FormulaType::_names()) {
    QString formula = QString::fromUtf8(name);
    if (formula != QStringLiteral("Invalid")) {
      result.append(formula);
    }
  }
  return result;
}

int main(int argc, char* argv[]) {
  resources::initialize();

  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  // Register types
  qRegisterMetaType<HistogramBuffer>();
  qRegisterMetaType<Point>();

  // Set up models

  auto* documentModel = new DocumentModel();
  QModelIndex blendIndex = documentModel->addBlend(FormulaType::DeJong);
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

  auto* engineManager = new EngineManager(&app);
  engineManager->setLoadUrl(resources::createUrl("forms/MainWindow.qml"));

  QObject::connect(engineManager, &EngineManager::engineAboutToBeCreated, [] {
    qmlRegisterType<DocumentModel>();
    qmlRegisterUncreatableType<DocumentEntryType>(
        "ChaosKit", 1, 0, "DocumentEntryType",
        QStringLiteral("Not creatable because it's an enum"));
    qmlRegisterInterface<ModelEntry>("ModelEntry");
    qmlRegisterType<SystemView>("ChaosKit", 1, 0, "SystemView");
  });

  auto onEngineCreated = [documentModel,
                          selectionModel](QQmlApplicationEngine* engine) {
    engine->addImportPath(resources::importPath());
    engine->addImageProvider(QStringLiteral("formula"),
                             new FormulaPreviewProvider);

    const QFont monospaceFont =
        QFontDatabase::systemFont(QFontDatabase::FixedFont);
    engine->rootContext()->setContextProperties({
        {QStringLiteral("formulaList"),
         QVariant::fromValue(createFormulaList())},
        {QStringLiteral("monospaceFont"), QVariant::fromValue(monospaceFont)},
        {QStringLiteral("documentModel"), QVariant::fromValue(documentModel)},
        {QStringLiteral("selectionModel"), QVariant::fromValue(selectionModel)},
    });
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
