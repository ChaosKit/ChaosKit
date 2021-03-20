#include "ProjectModel.h"

namespace chaoskit::ui {

ProjectModel::ProjectModel(QObject* parent)
    : QObject(parent), BaseModel<Project>() {
  colorMapModel_ = new ColorMapModel(this);
  systemModel_ = new SystemModel(this);
}

void ProjectModel::setProto(Project* proto) {
  BaseModel::setProto(proto);

  colorMapModel_->setProto(proto_->mutable_color_map());
  emit colorMapChanged();

  systemModel_->setProto(proto_->mutable_system());
  emit systemChanged();
}

void ProjectModel::setGamma(float gamma) {
  if (qFuzzyCompare(gamma, this->gamma())) return;

  proto_->set_gamma(gamma);
  emit gammaChanged();
}

void ProjectModel::setExposure(float exposure) {
  if (qFuzzyCompare(exposure, this->exposure())) return;

  proto_->set_exposure(exposure);
  emit exposureChanged();
}

void ProjectModel::setVibrancy(float vibrancy) {
  if (qFuzzyCompare(vibrancy, this->vibrancy())) return;

  proto_->set_vibrancy(vibrancy);
  emit vibrancyChanged();
}

void ProjectModel::setWidth(uint width) {
  if (width == this->width()) return;

  proto_->set_width(width);
  emit widthChanged();
}

void ProjectModel::setHeight(uint height) {
  if (height == this->height()) return;

  proto_->set_height(height);
  emit heightChanged();
}

}  // namespace chaoskit::ui
