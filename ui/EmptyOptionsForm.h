#ifndef CHAOSKIT_UI_EMPTYOPTIONSFORM_H
#define CHAOSKIT_UI_EMPTYOPTIONSFORM_H

#include <QWidget>

namespace Ui {
class EmptyOptionsForm;
}

namespace chaoskit {
namespace ui {

class EmptyOptionsForm : public QWidget {
  Q_OBJECT

 public:
  explicit EmptyOptionsForm(QWidget *parent = nullptr);
  ~EmptyOptionsForm();

 private:
  Ui::EmptyOptionsForm *ui;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_EMPTYOPTIONSFORM_H
