#include "Utilities.h"
#include <QDebug>
#include <QModelIndex>
#include <QUrl>

namespace chaoskit::ui {

QString Utilities::urlToLocalPath(const QUrl& url) { return url.toLocalFile(); }

QModelIndex Utilities::invalidModelIndex() { return QModelIndex(); }

}  // namespace chaoskit::ui
