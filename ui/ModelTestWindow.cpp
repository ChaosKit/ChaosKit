#include "ModelTestWindow.h"
#include <QAction>
#include <QDebug>
#include <QDir>
#include <QToolBar>
#include <QTreeView>

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
  auto* selectionModel = new QItemSelectionModel(model_, this);
  selectionModel->select(model_->systemIndex(),
                         QItemSelectionModel::ClearAndSelect);

  displayModel_ = new KSelectionProxyModel(selectionModel, this);
  displayModel_->setSourceModel(model_);

  treeView_ = new QTreeView(this);
  treeView_->setModel(displayModel_);

  setCentralWidget(treeView_);

  connect(
      treeView_->selectionModel(), &QItemSelectionModel::currentChanged,
      [this, addFormulaAction](const QModelIndex& current, const QModelIndex&) {
        addFormulaAction->setEnabled(
            model_->isBlend(displayModel_->mapToSource(current)));
      });
  connect(addBlendAction, &QAction::triggered, model_,
          &DocumentModel::addBlend);
  connect(addFormulaAction, &QAction::triggered, this,
          &ModelTestWindow::addFormula);
  connect(removeAction, &QAction::triggered, [this]() {
    auto index = displayModel_->mapToSource(treeView_->currentIndex());
    model_->removeRow(index.row(), index.parent());
  });
}

void ModelTestWindow::addFormula() {
  model_->addFormula(library::FormulaType::Linear,
                     displayModel_->mapToSource(treeView_->currentIndex()));
}

}  // namespace chaoskit::ui
