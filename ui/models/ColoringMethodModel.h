#ifndef CHAOSKIT_UI_MODELS_COLORINGMETHODMODEL_H
#define CHAOSKIT_UI_MODELS_COLORINGMETHODMODEL_H

#include <chaoskit.pb.h>
#include <QStringList>
#include "BaseModel.h"

namespace chaoskit::ui {

class ColoringMethodModel : public BaseModel<ColoringMethod> {
  Q_OBJECT
  Q_PROPERTY(const QStringList& types READ types CONSTANT FINAL);
  Q_PROPERTY(
      int typeIndex READ typeIndex WRITE setTypeIndex NOTIFY typeIndexChanged);
  Q_PROPERTY(float parameter READ parameter WRITE setParameter NOTIFY
                 parameterChanged);

 public:
  ColoringMethodModel(QObject* parent);
  void setProto(ColoringMethod* proto) override;

  [[nodiscard]] const QStringList& types() const;

  [[nodiscard]] int typeIndex() const;
  void setTypeIndex(int index);

  [[nodiscard]] float parameter() const;
  void setParameter(float param);

 signals:
  void typeIndexChanged();
  void parameterChanged();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_COLORINGMETHODMODEL_H
