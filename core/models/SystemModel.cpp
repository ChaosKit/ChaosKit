#include "SystemModel.h"

namespace chaoskit::core {

ModelCollection<BlendModel> SystemModel::blends() {
  return ModelCollection<BlendModel>(data_.lock()->blends);
}

BlendModel SystemModel::finalBlend() {
  return BlendModel(data_.lock()->finalBlend);
}

}  // namespace chaoskit::core
