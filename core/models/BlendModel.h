#ifndef CHAOSKIT_CORE_MODELS_BLENDMODEL_H
#define CHAOSKIT_CORE_MODELS_BLENDMODEL_H

#include "FormulaModel.h"
#include "Model.h"
#include "ModelCollection.h"
#include "core/structures/Blend.h"

namespace chaoskit::core {

class BlendModel : public Model<Blend> {
 public:
  CHAOSKIT_MODEL_CONSTRUCTORS(BlendModel);

  const std::string &name() const;
  void setName(const std::string &name);

  const Transform &pre() const;
  void setPre(const Transform &pre);
  const Transform &post() const;
  void setPost(const Transform &post);

  float weight() const;
  void setWeight(float weight);

  ModelCollection<FormulaModel> formulas();
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MODELS_BLENDMODEL_H
