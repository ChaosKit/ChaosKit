#ifndef CHAOSKIT_CORE_MODELS_SYSTEMMODEL_H
#define CHAOSKIT_CORE_MODELS_SYSTEMMODEL_H

#include <memory>
#include "BlendModel.h"
#include "Model.h"
#include "ModelCollection.h"
#include "core/structures/System.h"

namespace chaoskit::core {

class SystemModel : public Model<System> {
 public:
  CHAOSKIT_MODEL_CONSTRUCTORS(SystemModel)

  ModelCollection<BlendModel> blends();
  BlendModel finalBlend();
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MODELS_SYSTEMMODEL_H
