#ifndef CHAOSKIT_UI_COLORMAPREGISTRY_H
#define CHAOSKIT_UI_COLORMAPREGISTRY_H

#include <QObject>
#include "ColorMap.h"
#include "core/ColorMapRegistry.h"

namespace chaoskit::ui {

class ColorMapRegistry : public QObject {
  Q_OBJECT
 public:
  explicit ColorMapRegistry(QObject* parent = nullptr);
  void add(const QString& name, std::unique_ptr<core::ColorMap> colorMap);
  [[nodiscard]] const core::ColorMap* get(const QString& name) const;

 private:
  core::ColorMapRegistry registry_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_COLORMAPREGISTRY_H
