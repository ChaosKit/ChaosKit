#ifndef CHAOSKIT_UI_MODELTESTWINDOW_H
#define CHAOSKIT_UI_MODELTESTWINDOW_H

#include <KSelectionProxyModel>
#include <QMainWindow>
#include <QTreeView>
#include "DocumentModel.h"
#include "SubtreeModel.h"

namespace chaoskit::ui {

class ModelTestWindow : public QMainWindow {
  Q_OBJECT
 public:
  ModelTestWindow();

 private:
  DocumentModel* model_;
  KSelectionProxyModel* displayModel_;
  QTreeView* treeView_;

 private slots:
  void addFormula();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELTESTWINDOW_H
