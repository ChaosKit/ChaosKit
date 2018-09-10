#include <QApplication>
#include <QSurfaceFormat>
#include "HistogramBuffer.h"
#include "MainWindow.h"
#include "Point.h"
#include "System.h"
#include "SystemView.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::core::System;
using chaoskit::ui::SystemView;

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  qRegisterMetaType<HistogramBuffer>();
  qRegisterMetaType<Point>();
  qRegisterMetaType<System>();

  qmlRegisterType<SystemView>("app.chaoskit", 1, 0, "SystemView");

  chaoskit::ui::MainWindow window;
  window.show();

  return QApplication::exec();
}
