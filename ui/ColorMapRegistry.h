#ifndef CHAOSKIT_UI_COLORMAPREGISTRY_H
#define CHAOSKIT_UI_COLORMAPREGISTRY_H

#include <QObject>
#include "ColorMap.h"
#include "core/ColorMapRegistry.h"

namespace chaoskit::ui {

class ColorMapRegistry : public QObject {
  Q_OBJECT
  Q_PROPERTY(QStringList names READ names NOTIFY namesChanged)
 public:
  explicit ColorMapRegistry(QObject* parent = nullptr);
  void add(const QString& name, std::unique_ptr<core::ColorMap> colorMap);
  [[nodiscard]] const core::ColorMap* get(const std::string& name) const;
  [[nodiscard]] const core::ColorMap* get(const QString& name) const;
  [[nodiscard]] const QStringList& names() const { return nameCache_; }
  [[nodiscard]] const QString& defaultName() const;

 signals:
  void namesChanged();

 private:
  core::ColorMapRegistry registry_;
  QStringList nameCache_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_COLORMAPREGISTRY_H
