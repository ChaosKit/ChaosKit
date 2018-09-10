#ifndef CHAOSKIT_UI_MAINWINDOW_H
#define CHAOSKIT_UI_MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace chaoskit {
namespace ui {

class RenderingForm;
class SystemView;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private:
  Ui::MainWindow *ui;
  RenderingForm *renderingForm_;

  SystemView *findSystemView() const;

 private slots:
  void forwardRunStatus();
  void toggleGeneration();
};

#endif  // CHAOSKIT_UI_MAINWINDOW_H

}  // namespace ui
}  // namespace chaoskit
