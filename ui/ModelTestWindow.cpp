#include "ModelTestWindow.h"
#include <QAction>
#include <QDebug>
#include <QDir>
#include <QToolBar>
#include <QTreeView>
#include "DocumentModel.h"

namespace chaoskit::ui {

ModelTestWindow::ModelTestWindow() : QMainWindow(nullptr) {
  setWindowTitle(QStringLiteral("How the model looks like"));
  setFixedSize(640, 480);

  auto* toolbar = addToolBar("Toolbar");
  auto* addBlendAction = new QAction("Add Blend", this);
  toolbar->addAction(addBlendAction);
  auto* addFormulaAction = new QAction("Add Formula", this);
  addFormulaAction->setEnabled(false);
  toolbar->addAction(addFormulaAction);
  auto* removeAction = new QAction("Remove", this);
  toolbar->addAction(removeAction);

  model_ = new DocumentModel(this);

  treeView_ = new QTreeView(this);
  treeView_->setModel(model_);

  setCentralWidget(treeView_);

  connect(
      treeView_->selectionModel(), &QItemSelectionModel::currentChanged,
      [this, addFormulaAction](const QModelIndex& current, const QModelIndex&) {
        addFormulaAction->setEnabled(model_->isBlend(current));
      });
  connect(addBlendAction, &QAction::triggered, model_,
          &DocumentModel::addBlend);
  connect(addFormulaAction, &QAction::triggered, this,
          &ModelTestWindow::addFormula);
  connect(removeAction, &QAction::triggered, [this]() {
    auto index = treeView_->currentIndex();
    model_->removeRow(index.row(), index.parent());
  });
}

void ModelTestWindow::addFormula() {
  model_->addFormula(library::FormulaType::Linear, treeView_->currentIndex());
}

}  // namespace chaoskit::ui
