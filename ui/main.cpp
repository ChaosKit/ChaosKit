#include <library/FormulaType.h>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include "BlendModel.h"
#include "HistogramBuffer.h"
#include "Point.h"
#include "SystemElement.h"
#include "SystemModel.h"
#include "SystemView.h"
#include "models/System.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::ui::BlendModel;
using chaoskit::ui::System;
using chaoskit::ui::SystemElement;
using chaoskit::ui::SystemModel;
using chaoskit::ui::SystemView;
using chaoskit::library::FormulaType;

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

  qRegisterMetaType<HistogramBuffer>();
  qRegisterMetaType<Point>();

  qmlRegisterInterface<BlendModel>("BlendModel");
  qmlRegisterInterface<SystemElement>("SystemElement");

  qmlRegisterType<System>("app.chaoskit", 1, 0, "System");
  qmlRegisterType<SystemModel>("app.chaoskit", 1, 0, "SystemModel");
  qmlRegisterType<SystemView>("app.chaoskit", 1, 0, "SystemView");

  const QFont monospaceFont =
      QFontDatabase::systemFont(QFontDatabase::FixedFont);

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperties(
      {{QStringLiteral("formulaList"),
        QVariant::fromValue(createFormulaList())},
       {QStringLiteral("monospaceFont"), QVariant::fromValue(monospaceFont)}});
  engine.load(QUrl(QStringLiteral("qrc:/forms/MainWindow.qml")));

  return QGuiApplication::exec();
}
