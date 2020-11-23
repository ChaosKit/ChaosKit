#include "resources.h"
#include <QDir>

#define STRINGIFY(s) STRINGIFY_(s)
#define STRINGIFY_(s) #s

#ifdef CHAOSKIT_USE_RESOURCES
inline void init_() { Q_INIT_RESOURCE(resources); }
#endif

namespace chaoskit::resources {

void initialize() {
#ifdef CHAOSKIT_USE_RESOURCES
  init_();
#endif
}

#ifdef CHAOSKIT_USE_RESOURCES
bool areStatic() { return true; }
QString createPath(const QString& path) {
  return QStringLiteral(":/").append(path);
}
QUrl createUrl(const QString& path) {
  return QUrl(QStringLiteral("qrc:/").append(path));
}
QString importPath() { return QStringLiteral(":/"); }
#else
bool areStatic() { return false; }
QString createPath(const QString& path) {
  return QDir(STRINGIFY(CHAOSKIT_RESOURCE_DIR))
      .filePath(path);
}
QUrl createUrl(const QString& path) {
  return QUrl::fromLocalFile(createPath(path));
}
QString importPath() {
  return QDir::fromNativeSeparators(
      QStringLiteral(STRINGIFY(CHAOSKIT_RESOURCE_DIR)));
}
#endif

}  // namespace chaoskit::resources
