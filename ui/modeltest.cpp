#include <QApplication>
#include <QTimer>
#include "GlobalStore.h"
#include "ModelTestWindow.h"

using chaoskit::ui::GlobalStore;
using chaoskit::ui::ModelTestWindow;

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);

  auto* timer = new QTimer();
  QObject::connect(timer, &QTimer::timeout, &GlobalStore::get(),
                   &GlobalStore::propagateChanges);
  timer->start();

  auto* window = new ModelTestWindow;
  window->show();

  return QApplication::exec();
}
