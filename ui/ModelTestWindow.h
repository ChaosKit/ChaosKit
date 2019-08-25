#ifndef CHAOSKIT_UI_MODELTESTWINDOW_H
#define CHAOSKIT_UI_MODELTESTWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include "DocumentModel.h"

namespace chaoskit::ui {

class ModelTestWindow : public QMainWindow {
  Q_OBJECT
 public:
  ModelTestWindow();

 private:
  DocumentModel* model_;
  QTreeView* treeView_;

 private slots:
  void addFormula();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELTESTWINDOW_H
