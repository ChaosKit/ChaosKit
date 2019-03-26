#ifndef CHAOSKIT_UI_SYSTEMELEMENT_H
#define CHAOSKIT_UI_SYSTEMELEMENT_H

#include <QPersistentModelIndex>
#include <QQmlPropertyMap>

namespace chaoskit {
namespace ui {

class SystemModel;

class SystemElement : public QQmlPropertyMap {
 public:
  SystemElement(SystemModel *model, const QModelIndex &index);
  void proxyElement(QObject *element);

 private:
  SystemModel *model_;
  QPersistentModelIndex index_;
  QHash<QString, int> reverseRoleNames_;
  QObject *element_;

 private slots:
  void handleValueChanged(const QString &key, const QVariant &value);
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_SYSTEMELEMENT_H
