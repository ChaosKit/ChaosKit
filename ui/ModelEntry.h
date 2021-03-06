#ifndef CHAOSKIT_UI_MODELENTRY_H
#define CHAOSKIT_UI_MODELENTRY_H

#include <QAbstractItemModel>
#include <QHash>
#include <QPersistentModelIndex>
#include <QQmlPropertyMap>

namespace chaoskit::ui {

class ModelEntry : public QQmlPropertyMap {
  Q_OBJECT
 public:
  ModelEntry(QAbstractItemModel *model, const QModelIndex &index);

 private:
  QAbstractItemModel *model_;
  QPersistentModelIndex index_;
  QHash<QString, int> reverseRoleNames_;

 private slots:
  void handleDataChanged(const QModelIndex &topLeft,
                         const QModelIndex &bottomRight,
                         const QVector<int> &roles);
  void handleValueChanged(const QString &key, const QVariant &value);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELENTRY_H
