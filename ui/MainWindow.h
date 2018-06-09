#ifndef CHAOSKIT_UI_MAINWINDOW_H
#define CHAOSKIT_UI_MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace chaoskit {
namespace ui {

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

 private:
  Ui::MainWindow* ui;
};

#endif  // CHAOSKIT_UI_MAINWINDOW_H

}  // namespace ui
}  // namespace chaoskit
