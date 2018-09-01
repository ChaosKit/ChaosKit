#include <QApplication>
#include "HistogramBuffer.h"
#include "Point.h"
#include "System.h"
#include "TestWindow.h"

using chaoskit::core::HistogramBuffer;
using chaoskit::core::Point;
using chaoskit::core::System;

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  qRegisterMetaType<HistogramBuffer>();
  qRegisterMetaType<Point>();
  qRegisterMetaType<System>();

  chaoskit::ui::TestWindow window;
  window.show();

  return QApplication::exec();
}
