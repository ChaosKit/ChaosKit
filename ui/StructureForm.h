#ifndef CHAOSKIT_UI_STRUCTUREFORM_H
#define CHAOSKIT_UI_STRUCTUREFORM_H

#include <QDockWidget>
#include "SystemModel.h"

namespace Ui {
class StructureForm;
}

namespace chaoskit {
namespace ui {

class StructureForm : public QDockWidget {
  Q_OBJECT

 public:
  explicit StructureForm(QWidget *parent = nullptr);
  ~StructureForm();

 private:
  SystemModel *systemModel_;
  ::Ui::StructureForm *ui;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_STRUCTUREFORM_H
