#ifndef CHAOSKIT_UI_SYSTEMVIEW_H
#define CHAOSKIT_UI_SYSTEMVIEW_H

#include <QQuickFramebufferObject>
#include "ColorMapRegistry.h"
#include "DocumentModel.h"
#include "HistogramGenerator.h"

namespace chaoskit::ui {

class SystemView : public QQuickFramebufferObject {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::DocumentModel *model READ model WRITE setModel NOTIFY
                 modelChanged)
  Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
  Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY gammaChanged)
  Q_PROPERTY(
      float exposure READ exposure WRITE setExposure NOTIFY exposureChanged)
  Q_PROPERTY(
      float vibrancy READ vibrancy WRITE setVibrancy NOTIFY vibrancyChanged)
  Q_PROPERTY(ColorMapRegistry *colorMapRegistry READ colorMapRegistry WRITE
                 setColorMapRegistry NOTIFY colorMapRegistryChanged)
  Q_PROPERTY(
      QString colorMap READ colorMap WRITE setColorMap NOTIFY colorMapChanged)
 public:
  explicit SystemView(QQuickItem *parent = nullptr);

  Renderer *createRenderer() const override;
  void withHistogram(
      const std::function<void(const core::HistogramBuffer &)> &action) const;

  DocumentModel *model() const { return model_; }
  float gamma() const { return gamma_; }
  float exposure() const { return exposure_; }
  float vibrancy() const { return vibrancy_; }
  bool running() const { return generator_->running(); }
  [[nodiscard]] ColorMapRegistry *colorMapRegistry() {
    return colorMapRegistry_;
  }
  [[nodiscard]] const QString &colorMap() const { return colorMap_; }

 public slots:
  void start();
  void stop();
  void clear();
  void setRunning(bool running);
  void setModel(DocumentModel *documentModel);
  void setGamma(float gamma);
  void setExposure(float exposure);
  void setVibrancy(float vibrancy);
  void setColorMapRegistry(ColorMapRegistry *colorMapRegistry);
  void setColorMap(const QString &name);

 signals:
  void runningChanged();
  void modelChanged();
  void gammaChanged();
  void exposureChanged();
  void vibrancyChanged();
  void colorMapRegistryChanged();
  void colorMapChanged();

 private:
  HistogramGenerator *generator_;
  DocumentModel *model_ = nullptr;

  float gamma_ = 2.2f;
  float exposure_ = 0.f;
  float vibrancy_ = 0.f;
  ColorMapRegistry *colorMapRegistry_ = nullptr;
  QString colorMap_ = "Rainbow";

 private slots:
  void updateColorMap();
  void updateSystem();
  void updateBufferSize();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_SYSTEMVIEW_H
