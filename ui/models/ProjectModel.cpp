#include "ProjectModel.h"

#include <QFile>
#include <QLoggingCategory>
#include <QUrl>

namespace chaoskit::ui {

namespace {

Q_LOGGING_CATEGORY(modelLog, "ProjectModel");

}

ProjectModel::ProjectModel(ModelFactory* modelFactory, QObject* parent)
    : BaseModel<Project>(parent) {
  setObjectName("project");

  colorMapModel_ = modelFactory->createColorMapModel(this);
  systemModel_ = modelFactory->createSystemModel(this);

  // Emit "changed" signals for all properties when the project itself changes.
  connect(this, &ProjectModel::projectChanged, this,
          &ProjectModel::gammaChanged);
  connect(this, &ProjectModel::projectChanged, this,
          &ProjectModel::exposureChanged);
  connect(this, &ProjectModel::projectChanged, this,
          &ProjectModel::vibrancyChanged);
  connect(this, &ProjectModel::projectChanged, this,
          &ProjectModel::widthChanged);
  connect(this, &ProjectModel::projectChanged, this,
          &ProjectModel::colorMapChanged);
  connect(this, &ProjectModel::projectChanged, this,
          &ProjectModel::systemChanged);

  // Listen to proto changes in children and emit protoChanged.
  connect(colorMapModel_, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);
  connect(systemModel_, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);
  // Also emit systemChanged if the system proto changes.
  connect(systemModel_, &AbstractBaseModel::protoChanged, this,
          &ProjectModel::systemChanged);

  // Mark the model as modified if dependencies change.
  connect(colorMapModel_, &ColorMapModel::nameChanged, this,
          &ProjectModel::markAsModified);

  // Log the fileIoFailed signal.
  connect(this, &ProjectModel::fileIoFailed, [](const QString& error) {
    qCWarning(modelLog()) << "File IO failed:" << error;
  });
}

void ProjectModel::setProto(Project* proto) {
  auto* oldProto = proto_;
  BaseModel::setProto(proto);

  colorMapModel_->setProto(proto->mutable_color_map());
  systemModel_->setProto(proto->mutable_system());

  // Because Project is the root proto, release the old one once all child
  // models are updated with the new one. Deleting a null pointer is a no-op,
  // so no if is necessary.
  delete oldProto;

  // Will emit signals for all properties.
  emit projectChanged();

  emit protoChanged();
}

void ProjectModel::setGamma(float gamma) {
  if (qFuzzyCompare(gamma, this->gamma())) return;

  proto_->set_gamma(gamma);
  emit gammaChanged();
  emit protoChanged();
  markAsModified();
}

void ProjectModel::setExposure(float exposure) {
  if (qFuzzyCompare(exposure, this->exposure())) return;

  proto_->set_exposure(exposure);
  emit exposureChanged();
  emit protoChanged();
  markAsModified();
}

void ProjectModel::setVibrancy(float vibrancy) {
  if (qFuzzyCompare(vibrancy, this->vibrancy())) return;

  proto_->set_vibrancy(vibrancy);
  emit vibrancyChanged();
  emit protoChanged();
  markAsModified();
}

void ProjectModel::setWidth(uint width) {
  if (width == this->width()) return;

  proto_->set_width(width);
  emit widthChanged();
  emit protoChanged();
  markAsModified();
}

void ProjectModel::setHeight(uint height) {
  if (height == this->height()) return;

  proto_->set_height(height);
  emit heightChanged();
  emit protoChanged();
  markAsModified();
}

void ProjectModel::reset() {
  auto* project = new Project();

  // Set some sensible defaults.
  project->set_gamma(2.2f);
  project->set_exposure(0.f);
  project->set_vibrancy(0.f);
  project->set_width(1024);
  project->set_height(1024);

  auto* system = project->mutable_system();
  system->set_ttl(30);
  system->set_skip(0);
  system->mutable_final_blend()->set_enabled(true);

  auto* blend = system->add_blends();
  blend->set_enabled(true);
  blend->set_weight(1.f);
  blend->mutable_coloring_method()->set_single_color(.5f);

  setProto(project);
  clearFileUrl();
  markAsModified();
}

// File-related methods

void ProjectModel::setFileUrl(const QUrl& url) {
  if (fileUrl_ == url) return;
  name_ = url.fileName();
  fileUrl_ = url;
  emit nameChanged();
  emit fileUrlChanged();
}

void ProjectModel::clearFileUrl() {
  if (fileUrl_.isEmpty()) return;
  name_ = "Untitled";
  fileUrl_.clear();
  emit nameChanged();
  emit fileUrlChanged();
}

void ProjectModel::markAsModified() {
  if (modified_) return;
  modified_ = true;
  emit modifiedChanged();
}

void ProjectModel::markAsUnmodified() {
  if (!modified_) return;
  modified_ = false;
  emit modifiedChanged();
}

bool ProjectModel::loadFromUrl(const QUrl& url) {
  if (!url.isLocalFile()) {
    qCDebug(modelLog(), "Only URLs pointing to local files are allowed");
    return false;
  }

  QFile file(url.toLocalFile());
  if (!file.open(QIODevice::ReadOnly)) {
    emit fileIoFailed("Could not open the file");
    return false;
  }

  auto* proto = new Project();
  auto fileContents = file.readAll();
  if (!proto->ParseFromArray(fileContents.data(), fileContents.size())) {
    emit fileIoFailed("Could not parse the file");
    return false;
  }

  setProto(proto);
  setFileUrl(url);
  markAsUnmodified();
  return true;
}

bool ProjectModel::saveToUrl(const QUrl& url) {
  if (!url.isLocalFile()) {
    qCDebug(modelLog(), "Only URLs pointing to local files are allowed");
    return false;
  }

  QFile file(url.toLocalFile());
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    emit fileIoFailed("Could not open the file for writing");
    return false;
  }

  auto output = proto_->SerializeAsString();
  if (file.write(output.data(), output.size()) < 0) {
    emit fileIoFailed("Could not write the file");
    return false;
  }

  markAsUnmodified();
  setFileUrl(url);
  return true;
}

}  // namespace chaoskit::ui
