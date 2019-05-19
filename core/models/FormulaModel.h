#ifndef CHAOSKIT_CORE_MODELS_FORMULAMODEL_H
#define CHAOSKIT_CORE_MODELS_FORMULAMODEL_H

#include "Model.h"
#include "core/structures/Formula.h"

namespace chaoskit::core {

class FormulaModel : public Model<Formula> {
 public:
  CHAOSKIT_MODEL_CONSTRUCTORS(FormulaModel);

  const ast::Formula &source() const;
  void setSource(const ast::Formula &source);

  const std::vector<float> &params() const;
  void setParams(const std::vector<float> &params);

  const Formula::Weight &weight() const;
  void setWeight(float x, float y);
  void setWeight(float xy);
  void setWeight(const Formula::Weight &weight);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_MODELS_FORMULAMODEL_H
