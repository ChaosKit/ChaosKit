#include "SystemModel.h"
#include <google/protobuf/text_format.h>
#include <sstream>
#include "core/debug.h"
#include "ui/storage/conversions.h"

namespace chaoskit::ui {

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
