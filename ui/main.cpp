#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include "FlatteningModel.h"
#include "HistogramBuffer.h"
#include "MainWindow.h"
#include "Point.h"
#include "System.h"
#include "SystemView.h"
#include "SystemModel.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::core::System;
using chaoskit::ui::FlatteningModel;
using chaoskit::ui::SystemModel;
using chaoskit::ui::SystemView;

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
  qRegisterMetaType<System>();

  qmlRegisterInterface<FlatteningModel>("FlatteningModel");

  qmlRegisterType<SystemModel>("app.chaoskit", 1, 0, "SystemModel");
  qmlRegisterType<SystemView>("app.chaoskit", 1, 0, "SystemView");

  QQmlApplicationEngine engine;
  engine.load(QUrl(QStringLiteral("qrc:/MainWindow.qml")));

  return QGuiApplication::exec();
}
