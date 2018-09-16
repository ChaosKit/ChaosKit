#include "StructureForm.h"
#include "ui_StructureForm.h"

namespace chaoskit {
namespace ui {

StructureForm::StructureForm(QWidget *parent)
    : QDockWidget(parent), ui(new ::Ui::StructureForm) {
  ui->setupUi(this);

  systemModel_ = new SystemModel(this);
  ui->structureView->setModel(systemModel_);
}

StructureForm::~StructureForm() { delete ui; }

}  // namespace ui
}  // namespace chaoskit
