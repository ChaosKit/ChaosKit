#include <library/FormulaType.h>
#include <QDir>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QtGui/QTransform>
#include "DocumentModel.h"
#include "FormulaPreviewProvider.h"
#include "HistogramBuffer.h"
#include "ModelEntry.h"
#include "Point.h"
#include "SystemView.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::library::FormulaType;
using chaoskit::ui::DocumentEntryType;
using chaoskit::ui::DocumentModel;
using chaoskit::ui::FormulaPreviewProvider;
using chaoskit::ui::ModelEntry;
using chaoskit::ui::SystemView;

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

// Bypass QRC for a faster iteration cycle.
#ifdef CHAOSKIT_DEBUG
#define XSTR(s) STR(s)
#define STR(s) #s
QString createPath(const QString& path) {
  return QStringLiteral(XSTR(CHAOSKIT_SOURCE_DIR) "/ui/").append(path);
}
QUrl createUrl(const QString& path) {
  return QUrl::fromLocalFile(createPath(path));
}
#undef XSTR
#undef STR
#else
QString createPath(const QString& path) {
  return QStringLiteral(":/").append(path);
}
QUrl createUrl(const QString& path) {
  return QUrl(QStringLiteral("qrc:/").append(path));
}
#endif

int main(int argc, char* argv[]) {
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

  qmlRegisterType<DocumentModel>();
  qmlRegisterUncreatableType<DocumentEntryType>(
      "app.chaoskit", 1, 0, "DocumentEntryType",
      QStringLiteral("Not creatable because it's an enum"));
  qmlRegisterInterface<ModelEntry>("ModelEntry");
  qmlRegisterType<SystemView>("app.chaoskit", 1, 0, "SystemView");

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
  QFontDatabase::addApplicationFont(createPath("fonts/Inter-Regular.otf"));
  QFontDatabase::addApplicationFont(createPath("fonts/Inter-Medium.otf"));
  QFontDatabase::addApplicationFont(createPath("fonts/Inter-SemiBold.otf"));
  QFontDatabase::addApplicationFont(createPath("fonts/Inter-Bold.otf"));
  const QFont monospaceFont =
      QFontDatabase::systemFont(QFontDatabase::FixedFont);

  // Set up QML and pass data to the context
  QQmlApplicationEngine engine;
  engine.addImageProvider(QStringLiteral("formula"),
                          new FormulaPreviewProvider);
  engine.rootContext()->setContextProperties({
      {QStringLiteral("formulaList"), QVariant::fromValue(createFormulaList())},
      {QStringLiteral("monospaceFont"), QVariant::fromValue(monospaceFont)},
      {QStringLiteral("documentModel"), QVariant::fromValue(documentModel)},
      {QStringLiteral("selectionModel"), QVariant::fromValue(selectionModel)},
  });

#ifdef CHAOSKIT_DEBUG
  engine.rootContext()->setBaseUrl(createUrl(""));
#endif
  engine.load(createUrl("forms/MainWindow.qml"));

  return QGuiApplication::exec();
}
