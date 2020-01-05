#ifndef CHAOSKIT_UI_RESOURCES_H
#define CHAOSKIT_UI_RESOURCES_H

#include <QString>
#include <QUrl>

namespace chaoskit::resources {
  void initialize();
  bool areStatic();
  QString createPath(const QString& path);
  QUrl createUrl(const QString& path);
  QString importPath();
}

#endif  // CHAOSKIT_UI_RESOURCES_H
