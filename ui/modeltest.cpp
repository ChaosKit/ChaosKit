#include <QApplication>
#include "ModelTestWindow.h"

using chaoskit::ui::ModelTestWindow;

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);

  auto* window = new ModelTestWindow;
  window->show();

  return QApplication::exec();
}
