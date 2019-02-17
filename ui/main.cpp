#include <library/FormulaType.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include "FlatteningModel.h"
#include "HistogramBuffer.h"
#include "MainWindow.h"
#include "Point.h"
#include "SystemModel.h"
#include "SystemView.h"
#include "models/System.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::ui::FlatteningModel;
using chaoskit::ui::SystemModel;
using chaoskit::ui::SystemView;
using chaoskit::ui::models::System;
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

  qmlRegisterInterface<FlatteningModel>("FlatteningModel");

  qmlRegisterType<SystemModel>("app.chaoskit", 1, 0, "SystemModel");
  qmlRegisterType<SystemView>("app.chaoskit", 1, 0, "SystemView");
  qmlRegisterType<System>("app.chaoskit", 1, 0, "System");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty(
      QStringLiteral("formulaList"), QVariant::fromValue(createFormulaList()));
  engine.load(QUrl(QStringLiteral("qrc:/MainWindow.qml")));

  return QGuiApplication::exec();
}
