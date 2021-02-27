#ifndef CHAOSKIT_UI_MODELS_PROJECTMODEL_H
#define CHAOSKIT_UI_MODELS_PROJECTMODEL_H

#include <QObject>
#include "BaseModel.h"
#include "ColorMapModel.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class ProjectModel : public QObject, public BaseModel<Project> {
  Q_OBJECT
  Q_PROPERTY(ColorMapModel* colorMap READ colorMap);
  Q_PROPERTY(float gamma READ gamma WRITE setGamma NOTIFY gammaChanged);
  Q_PROPERTY(
      float exposure READ exposure WRITE setExposure NOTIFY exposureChanged);
  Q_PROPERTY(
      float vibrancy READ vibrancy WRITE setVibrancy NOTIFY vibrancyChanged);
  Q_PROPERTY(uint width READ width WRITE setWidth NOTIFY widthChanged);
  Q_PROPERTY(uint height READ height WRITE setHeight NOTIFY heightChanged);

 public:
  ProjectModel(QObject* parent = nullptr)
      : QObject(parent), BaseModel<Project>() {}

  ColorMapModel* colorMap();

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

 private:
  ColorMapModel* colorMapModel_ = nullptr;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_PROJECTMODEL_H
