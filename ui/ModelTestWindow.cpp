#include "ModelTestWindow.h"
#include <QDir>
#include <QFileSystemModel>
#include <QTreeView>

namespace chaoskit::ui {

ModelTestWindow::ModelTestWindow() : QMainWindow(nullptr) {
  setWindowTitle(QStringLiteral("How the model looks like"));
  setFixedSize(640, 480);

  auto* model = new QFileSystemModel;
  model->setRootPath(QDir::homePath());

  auto* treeView = new QTreeView(this);
  treeView->setModel(model);

  setCentralWidget(treeView);
}

}  // namespace chaoskit::ui
