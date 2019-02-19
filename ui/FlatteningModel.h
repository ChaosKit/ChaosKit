#ifndef CHAOSKIT_UI_FLATTENINGMODEL_H
#define CHAOSKIT_UI_FLATTENINGMODEL_H

#include <QAbstractProxyModel>

namespace chaoskit {
namespace ui {

class FlatteningModel : public QAbstractProxyModel {
  Q_OBJECT
  Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex NOTIFY
                 rootIndexChanged)
 public:
  void setSourceModel(QAbstractItemModel *model) override;
  QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
  QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent) const override;
  int columnCount(const QModelIndex &parent) const override;
  QModelIndex rootIndex() const { return rootIndex_; }
  void setRootIndex(const QModelIndex &index);

 signals:
  void rootIndexChanged();

 private:
  QPersistentModelIndex rootIndex_ = QPersistentModelIndex();

 private slots:
  void sourceDataChanged(const QModelIndex &topLeft,
                         const QModelIndex &bottomRight,
                         const QVector<int> &roles);
  void sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start,
                                   int end);
  void sourceRowsInserted(const QModelIndex &sourceParent, int start, int end);
  void sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start,
                                  int end);
  void sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end);
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_FLATTENINGMODEL_H
