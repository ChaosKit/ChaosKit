#include "Utilities.h"
#include <QDebug>
#include <QUrl>

namespace chaoskit::ui {

QString Utilities::urlToLocalPath(const QUrl& url) { return url.toLocalFile(); }

}  // namespace chaoskit::ui
