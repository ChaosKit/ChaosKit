#ifndef CHAOSKIT_CORE_MODELS_DOCUMENTMODEL_H
#define CHAOSKIT_CORE_MODELS_DOCUMENTMODEL_H

#include <memory>
#include "SystemModel.h"
#include "core/structures/Document.h"

namespace chaoskit::core {

class DocumentModel {
  std::unique_ptr<Document> data_;

 public:
  DocumentModel();

  SystemModel system();
  void resetSystem();
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MODELS_DOCUMENTMODEL_H
