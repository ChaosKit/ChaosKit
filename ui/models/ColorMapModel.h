#ifndef CHAOSKIT_UI_MODELS_COLORMAPMODEL_H
#define CHAOSKIT_UI_MODELS_COLORMAPMODEL_H

#include <QObject>
#include <optional>
#include "BaseModel.h"
#include "ColorMapRegistry.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class ColorMapModel : public QObject, public BaseModel<ColorMap> {
  Q_OBJECT
  Q_PROPERTY(const QString& name READ name WRITE setName NOTIFY nameChanged);

 public:
  ColorMapModel(QObject* parent = nullptr)
      : QObject(parent), BaseModel<ColorMap>() {}

  void setProto(ColorMap* proto) override;
  void setColorMapRegistry(ColorMapRegistry* registry);

  [[nodiscard]] const QString& name() const { return *nameCache_; }
  void setName(const QString& name);

  [[nodiscard]] const core::ColorMap* coreColorMap() const { return colorMap_; }

 signals:
  void nameChanged();
  void invalidNamePicked();

 private:
  std::optional<QString> nameCache_ = std::nullopt;
  ColorMapRegistry* colorMapRegistry_ = nullptr;
  const core::ColorMap* colorMap_ = nullptr;

  void updateNameCache();
  void updateColorMap();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_COLORMAPMODEL_H
