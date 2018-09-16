#include "EmptyOptionsForm.h"
#include "ui_EmptyOptionsForm.h"

namespace chaoskit {
namespace ui {

EmptyOptionsForm::EmptyOptionsForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::EmptyOptionsForm) {
  ui->setupUi(this);
}

EmptyOptionsForm::~EmptyOptionsForm() { delete ui; }

}  // namespace ui
}  // namespace chaoskit
