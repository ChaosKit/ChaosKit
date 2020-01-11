#ifndef CHAOSKIT_UI_ENGINEMANAGER_H
#define CHAOSKIT_UI_ENGINEMANAGER_H

#include <QFileSystemWatcher>
#include <QObject>
#include <QQmlApplicationEngine>

namespace chaoskit::ui {

class EngineManager : public QObject {
  Q_OBJECT
 public:
  EngineManager(QObject* parent = nullptr);
  void setLoadUrl(const QUrl& url);
  void enableHotReload();
  void addWatchDirectory(const QString& directory);
  void load();

 signals:
  void engineAboutToBeCreated();
  void engineCreated(QQmlApplicationEngine* engine);

 private:
  QQmlApplicationEngine* engine_;
  QFileSystemWatcher* watcher_;
  QUrl loadUrl_;

  void createEngine();

 private slots:
  void reload(const QString& changedPath);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_ENGINEMANAGER_H
