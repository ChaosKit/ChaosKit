#include <QApplication>
#include <QPushButton>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  chaoskit::ui::MainWindow window;
  window.show();

  return QApplication::exec();
}
