#include "ModelFactory.h"

#include "BlendModel.h"
#include "ColorMapModel.h"
#include "FormulaModel.h"
#include "ProjectModel.h"
#include "SystemModel.h"
#include "TransformModel.h"

namespace chaoskit::ui {

BlendModel* ModelFactory::createBlendModel(QObject* parent) {
  return new BlendModel(this, parent);
}
ColorMapModel* ModelFactory::createColorMapModel(QObject* parent) {
  return new ColorMapModel(parent);
}
FormulaModel* ModelFactory::createFormulaModel(QObject* parent) {
  return new FormulaModel(parent);
}
ProjectModel* ModelFactory::createProjectModel(QObject* parent) {
  return new ProjectModel(this, parent);
}
SystemModel* ModelFactory::createSystemModel(QObject* parent) {
  return new SystemModel(this, parent);
}
TransformModel* ModelFactory::createTransformModel(QObject* parent) {
  return new TransformModel(parent);
}

}  // namespace chaoskit::ui
