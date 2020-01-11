#include "EngineManager.h"
#include <QDir>
#include <QLoggingCategory>
#include <QQuickWindow>
#include <QStack>
#include <QStringList>

namespace chaoskit::ui {

Q_LOGGING_CATEGORY(engineManager, "EngineManager");

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

EngineManager::EngineManager(QObject* parent)
    : QObject(parent), engine_(nullptr), watcher_(nullptr), loadUrl_() {}

void EngineManager::createEngine() {
  emit engineAboutToBeCreated();
  engine_ = new QQmlApplicationEngine(this);
  emit engineCreated(engine_);
}

void EngineManager::setLoadUrl(const QUrl& url) { loadUrl_ = url; }

void EngineManager::load() {
  Q_ASSERT(!loadUrl_.isEmpty());
  if (engine_ == nullptr) {
    createEngine();
  }

  engine_->load(loadUrl_);
}

void EngineManager::enableHotReload() {
  watcher_ = new QFileSystemWatcher(this);

  // FIXME: On macOS editing a file triggers a directory update as well.
  // Fix this properly to watch additions and removals instead of only changes.
  //  connect(watcher_, &QFileSystemWatcher::directoryChanged, this,
  //          &HotReloader::reload);
  connect(watcher_, &QFileSystemWatcher::fileChanged, this,
          &EngineManager::reload);
}

void EngineManager::addWatchDirectory(const QString& directory) {
  Q_ASSERT(watcher_ != nullptr);
  watcher_->addPaths(getPathsToWatch(directory));
}

void EngineManager::reload(const QString& changedPath) {
  qDebug(engineManager) << "Reloading due to path change:" << changedPath;

  delete engine_;
  qmlClearTypeRegistrations();
  createEngine();
  load();
}

}  // namespace chaoskit::ui
