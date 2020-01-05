#ifndef CHAOSKIT_UI_HOTRELOADER_H
#define CHAOSKIT_UI_HOTRELOADER_H

#include <QFileSystemWatcher>
#include <QObject>
#include <QQmlApplicationEngine>

namespace chaoskit::ui {

class HotReloader : public QObject {
  Q_OBJECT
 public:
  HotReloader(QQmlApplicationEngine* engine, QObject* parent = nullptr);
  void addWatchDirectory(const QString& directory);
  void setReloadPath(const QString& path);

 private:
  QQmlApplicationEngine* engine_;
  QFileSystemWatcher* watcher_;
  QString reloadPath_;

 private slots:
  void reload(const QString& changedPath);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_HOTRELOADER_H
