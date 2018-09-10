#ifndef CHAOSKIT_UI_RENDERINGFORM_H
#define CHAOSKIT_UI_RENDERINGFORM_H

#include <QDockWidget>

namespace Ui {
class RenderingForm;
}

namespace chaoskit {
namespace ui {

class RenderingForm : public QDockWidget {
  Q_OBJECT

 public:
  explicit RenderingForm(QWidget* parent = nullptr);
  ~RenderingForm();

 public slots:
  void updateStartStopLabel(bool running);

 signals:
  void startStopClicked();
  void clearClicked();
  void gammaChanged(float gamma);
  void exposureChanged(float exposure);
  void vibrancyChanged(float vibrancy);

 private:
  ::Ui::RenderingForm* ui;

 private slots:
  void forwardGamma(int gamma);
  void forwardExposure(int exposure);
  void forwardVibrancy(int vibrancy);
  void resetParameters();
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_RENDERINGFORM_H
