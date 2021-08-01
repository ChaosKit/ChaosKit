#ifndef CHAOSKIT_UI_MODELS_COLORMAPMODEL_H
#define CHAOSKIT_UI_MODELS_COLORMAPMODEL_H

#include <QColor>
#include <QObject>
#include <optional>
#include "BaseModel.h"
#include "ColorMapRegistry.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class ColorMapModel : public BaseModel<ColorMap> {
  Q_OBJECT
  Q_PROPERTY(const QString& name READ name WRITE setName NOTIFY nameChanged);
  Q_PROPERTY(int index READ index NOTIFY indexChanged STORED false);
  Q_PROPERTY(ColorMapRegistry* registry READ registry WRITE setColorMapRegistry
                 NOTIFY registryChanged);

 public:
  ColorMapModel(QObject* parent = nullptr);

  void setProto(ColorMap* proto) override;

  [[nodiscard]] ColorMapRegistry* registry() const { return colorMapRegistry_; }
  void setColorMapRegistry(ColorMapRegistry* registry);

  [[nodiscard]] const QString& name() const { return *nameCache_; }
  void setName(const QString& name);

  [[nodiscard]] int index() const;

  [[nodiscard]] const core::ColorMap* coreColorMap() const { return colorMap_; }

  [[nodiscard]] Q_INVOKABLE QColor colorAt(float position) const;

 signals:
  void nameChanged();
  void indexChanged();
  void registryChanged();
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
