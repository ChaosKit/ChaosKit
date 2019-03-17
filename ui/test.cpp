#include <QApplication>
#include <QSurfaceFormat>
#include "TestWindow.h"

using chaoskit::ui::TestWindow;

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  auto *window = new TestWindow();
  window->show();

  return QApplication::exec();
}
