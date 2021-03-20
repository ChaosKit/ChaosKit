#ifndef CHAOSKIT_UI_MODELS_PROJECTMODEL_H
#define CHAOSKIT_UI_MODELS_PROJECTMODEL_H

#include <QObject>
#include "BaseModel.h"
#include "ColorMapModel.h"
#include "SystemModel.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class ProjectModel : public QObject, public BaseModel<Project> {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::ColorMapModel* colorMap READ colorMap NOTIFY
                 colorMapChanged);
  Q_PROPERTY(
      chaoskit::ui::SystemModel* system READ system NOTIFY systemChanged);
  Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY gammaChanged);
  Q_PROPERTY(
      float exposure READ exposure WRITE setExposure NOTIFY exposureChanged);
  Q_PROPERTY(
      float vibrancy READ vibrancy WRITE setVibrancy NOTIFY vibrancyChanged);
  Q_PROPERTY(uint width READ width WRITE setWidth NOTIFY widthChanged);
  Q_PROPERTY(uint height READ height WRITE setHeight NOTIFY heightChanged);

 public:
  ProjectModel(QObject* parent = nullptr);

  void setProto(Project* proto) override;

  [[nodiscard]] ColorMapModel* colorMap() const { return colorMapModel_; }
  [[nodiscard]] SystemModel* system() const { return systemModel_; }

  [[nodiscard]] float gamma() const { return proto_->gamma(); }
  void setGamma(float gamma);

  [[nodiscard]] float exposure() const { return proto_->exposure(); }
  void setExposure(float exposure);

  [[nodiscard]] float vibrancy() const { return proto_->vibrancy(); }
  void setVibrancy(float vibrancy);

  [[nodiscard]] uint width() const { return proto_->width(); }
  void setWidth(uint width);

  [[nodiscard]] uint height() const { return proto_->height(); }
  void setHeight(uint height);

 signals:
  void gammaChanged();
  void exposureChanged();
  void vibrancyChanged();
  void widthChanged();
  void heightChanged();
  void colorMapChanged();
  void systemChanged();

 private:
  ColorMapModel* colorMapModel_;
  SystemModel* systemModel_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_PROJECTMODEL_H
