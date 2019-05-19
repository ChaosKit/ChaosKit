#include "BlendModel.h"

namespace chaoskit::core {

const std::string& BlendModel::name() const { return data_.lock()->name; }
void BlendModel::setName(const std::string& name) { data_.lock()->name = name; }

const Transform& BlendModel::pre() const { return data_.lock()->pre; }
void BlendModel::setPre(const Transform& pre) { data_.lock()->pre = pre; }
const Transform& BlendModel::post() const { return data_.lock()->post; }
void BlendModel::setPost(const Transform& post) { data_.lock()->post = post; }

float BlendModel::weight() const { return data_.lock()->weight; }
void BlendModel::setWeight(float weight) { data_.lock()->weight = weight; }

ModelCollection<FormulaModel> BlendModel::formulas() {
  return ModelCollection<FormulaModel>(data_.lock()->formulas);
}

}  // namespace chaoskit::core
