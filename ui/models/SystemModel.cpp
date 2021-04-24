#include "SystemModel.h"
#include <google/protobuf/text_format.h>
#include <sstream>
#include "core/debug.h"
#include "ui/storage/conversions.h"

namespace chaoskit::ui {

SystemModel::SystemModel(ModelFactory* modelFactory, QObject* parent)
    : BaseModel<System>(parent), modelFactory_(modelFactory) {
  setObjectName("system");

  blends_ = new QQmlObjectListModel<BlendModel>(this);
  cameraBlend_ = modelFactory_->createBlendModel(this);

  // Listen to proto changes in children and emit protoChanged.
  connect(cameraBlend_, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);
}

void SystemModel::setProto(System* proto) {
  BaseModel::setProto(proto);

  // Clear old blend models
  for (auto* blend : *blends_) {
    blend->disconnect(this);
  }
  blends_->clear();

  // Create new blend models
  QList<BlendModel*> newModels;
  for (auto& blend : *proto_->mutable_blends()) {
    auto* model = modelFactory_->createBlendModel(blends_);
    model->setProto(&blend);
    connect(model, &AbstractBaseModel::protoChanged, this,
            &AbstractBaseModel::protoChanged);
    newModels.append(model);
  }
  blends_->append(newModels);

  // Populate the camera blend
  cameraBlend_->setProto(proto_->mutable_final_blend());

  emit protoChanged();
}

void SystemModel::addBlend() {
  auto* model = modelFactory_->createBlendModel(blends_);
  connect(model, &AbstractBaseModel::protoChanged, this,
          &AbstractBaseModel::protoChanged);

  Blend* blend = proto_->add_blends();
  blend->set_enabled(true);
  model->setProto(blend);

  blends_->append(model);
  emit protoChanged();
}

QString SystemModel::astSource() const {
  std::stringstream stream;
  const core::CameraSystem& cameraSystem = toCameraSystem(*proto_);
  stream << cameraSystem;
  return QString::fromStdString(stream.str());
}
QString SystemModel::modelSource() const {
  std::string output;
  google::protobuf::TextFormat::PrintToString(*proto_, &output);
  return QString::fromStdString(output);
}

}  // namespace chaoskit::ui
