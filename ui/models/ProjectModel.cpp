#include "ProjectModel.h"

namespace chaoskit::ui {

ColorMapModel* ProjectModel::colorMap() {
  if (!proto_->has_color_map()) return nullptr;

  if (colorMapModel_ == nullptr) {
    colorMapModel_ = new ColorMapModel(this);
    colorMapModel_->setProto(proto_->mutable_color_map());
  }

  return colorMapModel_;
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
