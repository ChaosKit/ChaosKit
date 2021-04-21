#ifndef CHAOSKIT_UI_MODELS_MODELFACTORY_H
#define CHAOSKIT_UI_MODELS_MODELFACTORY_H

#include <QObject>

namespace chaoskit::ui {

class BlendModel;
class ColorMapModel;
class ProjectModel;
class SystemModel;
class TransformModel;

class ModelFactory {
 public:
  virtual BlendModel* createBlendModel(QObject* parent);
  virtual ProjectModel* createProjectModel(QObject* parent);
  virtual ColorMapModel* createColorMapModel(QObject* parent);
  virtual SystemModel* createSystemModel(QObject* parent);
  virtual TransformModel* createTransformModel(QObject* parent);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_MODELFACTORY_H