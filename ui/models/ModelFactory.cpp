#include "ModelFactory.h"

#include "ColorMapModel.h"
#include "ProjectModel.h"
#include "SystemModel.h"

namespace chaoskit::ui {

ProjectModel* ModelFactory::createProjectModel(QObject* parent) {
  return new ProjectModel(this, parent);
}
ColorMapModel* ModelFactory::createColorMapModel(QObject* parent) {
  return new ColorMapModel(parent);
}
SystemModel* ModelFactory::createSystemModel(QObject* parent) {
  return new SystemModel(parent);
}

}  // namespace chaoskit::ui
