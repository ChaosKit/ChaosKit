#include <library/FormulaType.h>
#include <KSelectionProxyModel>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QtGui/QTransform>
#include "DocumentModel.h"
#include "HistogramBuffer.h"
#include "ModelEntry.h"
#include "Point.h"
#include "SystemView.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::library::FormulaType;
using chaoskit::ui::DocumentEntryType;
using chaoskit::ui::DocumentModel;
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
  qmlRegisterType<KSelectionProxyModel>();
  qmlRegisterType<SystemView>("app.chaoskit", 1, 0, "SystemView");

  // Set up models

  auto* documentModel = new DocumentModel();
  QModelIndex blendIndex = documentModel->addBlend();
  QModelIndex formulaIndex =
      documentModel->addFormula(FormulaType::DeJong, blendIndex);
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

  // Set up QML and pass data to the context

  const QFont monospaceFont =
      QFontDatabase::systemFont(QFontDatabase::FixedFont);

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperties({
      {QStringLiteral("formulaList"), QVariant::fromValue(createFormulaList())},
      {QStringLiteral("monospaceFont"), QVariant::fromValue(monospaceFont)},
      {QStringLiteral("documentModel"), QVariant::fromValue(documentModel)},
      {QStringLiteral("selectionModel"), QVariant::fromValue(selectionModel)},
  });
  engine.load(QUrl(QStringLiteral("qrc:/forms/MainWindow.qml")));

  return QGuiApplication::exec();
}
