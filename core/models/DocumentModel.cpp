#include "DocumentModel.h"

namespace chaoskit::core {

DocumentModel::DocumentModel() : data_(std::make_unique<Document>()) {
  resetSystem();
}

SystemModel DocumentModel::system() { return SystemModel(data_->system); }

void DocumentModel::resetSystem() {
  auto system = std::make_shared<System>();
  system->finalBlend = std::make_shared<Blend>();
  data_->system = std::move(system);
}

}  // namespace chaoskit::core
