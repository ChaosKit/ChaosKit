#ifndef CHAOSKIT_UI_SYSTEMVIEW_H
#define CHAOSKIT_UI_SYSTEMVIEW_H

#include <QQuickFramebufferObject>
#include "HistogramGenerator.h"

namespace chaoskit {
namespace ui {

class SystemView : public QQuickFramebufferObject {
  Q_OBJECT
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

  int ttl() const { return ttl_; }
  void setTtl(int ttl);

  float gamma() const { return gamma_; }
  void setGamma(float gamma);

  float exposure() const { return exposure_; }
  void setExposure(float exposure);

  float vibrancy() const { return vibrancy_; }
  void setVibrancy(float vibrancy);

 signals:
  void ttlChanged();
  void gammaChanged();
  void exposureChanged();
  void vibrancyChanged();

 protected:
  void componentComplete() override;

 private:
  HistogramGenerator *generator_;
  int ttl_ = chaoskit::core::Particle::IMMORTAL;
  float gamma_ = 2.2f;
  float exposure_ = 0.f;
  float vibrancy_ = 0.f;

 private slots:
  void updateBufferSize();
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_SYSTEMVIEW_H
