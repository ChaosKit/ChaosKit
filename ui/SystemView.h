#ifndef CHAOSKIT_UI_SYSTEMVIEW_H
#define CHAOSKIT_UI_SYSTEMVIEW_H

#include <QQuickFramebufferObject>
#include "DocumentModel.h"
#include "HistogramGenerator.h"

namespace chaoskit::ui {

class SystemView : public QQuickFramebufferObject {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::System *system READ system WRITE setSystem NOTIFY
                 systemChanged)
  Q_PROPERTY(chaoskit::ui::DocumentModel *model READ model WRITE setModel NOTIFY
                 modelChanged)
  Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
  Q_PROPERTY(int ttl READ ttl WRITE setTtl NOTIFY ttlChanged)
  Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY gammaChanged)
  Q_PROPERTY(
      float exposure READ exposure WRITE setExposure NOTIFY exposureChanged)
  Q_PROPERTY(
      float vibrancy READ vibrancy WRITE setVibrancy NOTIFY vibrancyChanged)
 public:
  explicit SystemView(QQuickItem *parent = nullptr);

  Renderer *createRenderer() const override;
  void withHistogram(
      const std::function<void(const core::HistogramBuffer &)> &action) const;

  System *system() const { return system_; }
  DocumentModel *model() const { return model_; }
  int ttl() const { return ttl_; }
  float gamma() const { return gamma_; }
  float exposure() const { return exposure_; }
  float vibrancy() const { return vibrancy_; }
  bool running() const { return generator_->running(); }

 public slots:
  void start();
  void stop();
  void clear();
  void setRunning(bool running);
  void setSystem(System *system);
  void setModel(DocumentModel *documentModel);
  void setTtl(int ttl);
  void setGamma(float gamma);
  void setExposure(float exposure);
  void setVibrancy(float vibrancy);

 signals:
  void runningChanged();
  void systemChanged();
  void modelChanged();
  void ttlChanged();
  void gammaChanged();
  void exposureChanged();
  void vibrancyChanged();

 private:
  HistogramGenerator *generator_;
  DocumentModel *model_ = nullptr;
  System *system_ = nullptr;

  int ttl_ = chaoskit::core::Particle::IMMORTAL;
  float gamma_ = 2.2f;
  float exposure_ = 0.f;
  float vibrancy_ = 0.f;

 private slots:
  void updateSystem();
  void updateBufferSize();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_SYSTEMVIEW_H
