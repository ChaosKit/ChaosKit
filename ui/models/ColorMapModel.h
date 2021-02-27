#ifndef CHAOSKIT_UI_MODELS_COLORMAPMODEL_H
#define CHAOSKIT_UI_MODELS_COLORMAPMODEL_H

#include <QObject>
#include <optional>
#include "BaseModel.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class ColorMapModel : public QObject, public BaseModel<ColorMap> {
  Q_OBJECT
  Q_PROPERTY(const QString& name READ name WRITE setName NOTIFY nameChanged);

 public:
  ColorMapModel(QObject* parent = nullptr)
      : QObject(parent), BaseModel<ColorMap>() {}

  [[nodiscard]] const QString& name();
  void setName(const QString& name);

 public slots:
  void nameChanged();

 private:
  std::optional<QString> nameCache_ = std::nullopt;

  void ensureNameCacheIsFilled();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_COLORMAPMODEL_H
