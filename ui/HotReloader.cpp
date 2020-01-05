#include "HotReloader.h"
#include <QDir>
#include <QLoggingCategory>
#include <QQuickWindow>
#include <QStack>
#include <QStringList>

namespace chaoskit::ui {

Q_LOGGING_CATEGORY(hotreloader, "hotreloader");

namespace {

QStringList getPathsToWatch(const QString& directory) {
  QStringList pathsToWatch;

  QStack<QDir> dirStack{{QDir(directory)}};
  while (!dirStack.isEmpty()) {
    QDir dir = dirStack.pop();
    auto qmlFiles = dir.entryList({"*.qml"}, QDir::Files);

    // Watch the directory containing QML files to react to additions and
    // removals.
    if (!qmlFiles.isEmpty()) {
      pathsToWatch << dir.absolutePath();
    }

    // Watch files for changes.
    for (const QString& file : qmlFiles) {
      pathsToWatch << dir.absoluteFilePath(file);
    }

    // Recurse.
    for (const QString& subdir :
         dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
      dirStack.push(QDir(dir.absoluteFilePath(subdir)));
    }
  }

  return pathsToWatch;
}

}  // namespace

HotReloader::HotReloader(QQmlApplicationEngine* engine, QObject* parent)
    : QObject(parent), engine_(engine), watcher_(nullptr), reloadPath_() {
  watcher_ = new QFileSystemWatcher(this);

  // FIXME: On macOS editing a file triggers a directory update as well.
  // Fix this properly to watch additions and removals instead of only changes.
  //  connect(watcher_, &QFileSystemWatcher::directoryChanged, this,
  //          &HotReloader::reload);
  connect(watcher_, &QFileSystemWatcher::fileChanged, this,
          &HotReloader::reload);
}

void HotReloader::addWatchDirectory(const QString& directory) {
  watcher_->addPaths(getPathsToWatch(directory));
}

void HotReloader::setReloadPath(const QString& path) { reloadPath_ = path; }

void HotReloader::reload(const QString& changedPath) {
  if (reloadPath_.isEmpty()) {
    qWarning(hotreloader) << "NOT RELOADING because reloadPath is not set";
    return;
  }

  qDebug(hotreloader) << "Reloading due to path change:" << changedPath;

  auto* window = qobject_cast<QQuickWindow*>(engine_->rootObjects().first());

  engine_->clearComponentCache();
  engine_->load(reloadPath_);

  window->close();
  delete window;
}

}  // namespace chaoskit::ui
