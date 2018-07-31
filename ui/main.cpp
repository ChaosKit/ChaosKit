#include <QApplication>
#include <QPushButton>
#include "TestWindow.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  chaoskit::ui::TestWindow window;
  window.show();

  return QApplication::exec();
}
