#include "SystemModel.h"
#include <google/protobuf/text_format.h>
#include <sstream>
#include "core/debug.h"
#include "ui/storage/conversions.h"

namespace chaoskit::ui {

SystemModel::SystemModel(ModelFactory* modelFactory, QObject* parent)
    : QObject(parent), BaseModel<System>(), modelFactory_(modelFactory) {
  blends_ = new QQmlObjectListModel<BlendModel>(this);
}

void SystemModel::setProto(System* proto) {
  BaseModel::setProto(proto);

  // Clear old blend models
  blends_->clear();

  // Create new blend models
  QList<BlendModel*> newModels;
  for (auto& blend : *proto_->mutable_blends()) {
    auto* model = modelFactory_->createBlendModel(blends_);
    model->setProto(&blend);
    newModels.append(model);
  }
  blends_->append(newModels);
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
