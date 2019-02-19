#include "FlatteningModel.h"

namespace chaoskit {
namespace ui {

void FlatteningModel::setSourceModel(QAbstractItemModel *model) {
  beginResetModel();

  if (sourceModel()) {
    sourceModel()->disconnect(this);
  }

  QAbstractProxyModel::setSourceModel(model);

  if (model) {
    connect(model, &QAbstractItemModel::dataChanged, this,
            &FlatteningModel::sourceDataChanged);
    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this,
            &FlatteningModel::sourceRowsAboutToBeInserted);
    connect(model, &QAbstractItemModel::rowsInserted, this,
            &FlatteningModel::sourceRowsInserted);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            &FlatteningModel::sourceRowsAboutToBeRemoved);
    connect(model, &QAbstractItemModel::rowsRemoved, this,
            &FlatteningModel::sourceRowsRemoved);
  }

  resetInternalData();
  endResetModel();
}

void FlatteningModel::setRootIndex(const QModelIndex &index) {
  if (rootIndex_ == index) {
    return;
  }

  rootIndex_ = index;
  emit rootIndexChanged();
}

QModelIndex FlatteningModel::mapFromSource(
    const QModelIndex &sourceIndex) const {
  if (sourceModel() == nullptr || !sourceIndex.isValid() ||
      sourceIndex.parent() != rootIndex_) {
    return QModelIndex();
  }

  return createIndex(sourceIndex.row(), sourceIndex.column(),
                     sourceIndex.internalPointer());
}

QModelIndex FlatteningModel::mapToSource(const QModelIndex &proxyIndex) const {
  if (!proxyIndex.isValid()) {
    return rootIndex_;
  }

  return rootIndex_.child(proxyIndex.row(), proxyIndex.column());
}

QModelIndex FlatteningModel::index(int row, int column,
                                   const QModelIndex &parent) const {
  if (sourceModel() == nullptr) {
    return QModelIndex();
  }

  auto sourceParent = mapToSource(parent);
  auto sourceIndex = sourceModel()->index(row, column, sourceParent);
  return mapFromSource(sourceIndex);
}

QModelIndex FlatteningModel::parent(const QModelIndex &child) const {
  if (sourceModel() == nullptr) {
    return QModelIndex();
  }

  auto sourceIndex = mapToSource(child);
  auto sourceParent = sourceModel()->parent(sourceIndex);
  return mapFromSource(sourceParent);
}

int FlatteningModel::rowCount(const QModelIndex &parent) const {
  return sourceModel()->rowCount(mapToSource(parent));
}

int FlatteningModel::columnCount(const QModelIndex &parent) const {
  return sourceModel()->columnCount(mapToSource(parent));
}

void FlatteningModel::sourceDataChanged(const QModelIndex &topLeft,
                                        const QModelIndex &bottomRight,
                                        const QVector<int> &roles) {
  QModelIndex targetTopLeft = mapFromSource(topLeft);
  QModelIndex targetBottomRight = mapFromSource(bottomRight);

  if (!topLeft.isValid() || !bottomRight.isValid()) {
    return;
  }

  emit dataChanged(targetTopLeft, targetBottomRight, roles);
}

void FlatteningModel::sourceRowsAboutToBeInserted(
    const QModelIndex &sourceParent, int start, int end) {
  if (sourceParent != rootIndex_) {
    return;
  }

  beginInsertRows(QModelIndex(), start, end);
}

void FlatteningModel::sourceRowsInserted(const QModelIndex &sourceParent,
                                         int start, int end) {
  if (sourceParent != rootIndex_) {
    return;
  }

  endInsertRows();
}

void FlatteningModel::sourceRowsAboutToBeRemoved(
    const QModelIndex &sourceParent, int start, int end) {
  if (sourceParent != rootIndex_) {
    return;
  }

  beginRemoveRows(QModelIndex(), start, end);
}

void FlatteningModel::sourceRowsRemoved(const QModelIndex &sourceParent,
                                        int start, int end) {
  if (sourceParent != rootIndex_) {
    return;
  }

  endRemoveRows();
}

}  // namespace ui
}  // namespace chaoskit
