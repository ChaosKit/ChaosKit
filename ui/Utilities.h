#ifndef CHAOSKIT_UI_UTILITIES_H
#define CHAOSKIT_UI_UTILITIES_H

#include <QObject>

namespace chaoskit::ui {

class Utilities : public QObject {
  Q_OBJECT
 public:
  explicit Utilities(QObject* parent = nullptr) : QObject(parent) {}

  Q_INVOKABLE QString urlToLocalPath(const QUrl& url);
  Q_INVOKABLE QModelIndex invalidModelIndex();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_UTILITIES_H
