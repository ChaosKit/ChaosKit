#include "SubtreeModel.h"

namespace chaoskit::ui {

void SubtreeModel::setSourceModel(QAbstractItemModel *model) {
  beginResetModel();

  if (sourceModel()) {
    sourceModel()->disconnect(this);
  }

  QAbstractProxyModel::setSourceModel(model);

  if (model) {
    connect(model, &QAbstractItemModel::dataChanged, this,
            &SubtreeModel::sourceDataChanged);
    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this,
            &SubtreeModel::sourceRowsAboutToBeInserted);
    connect(model, &QAbstractItemModel::rowsInserted, this,
            &SubtreeModel::sourceRowsInserted);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            &SubtreeModel::sourceRowsAboutToBeRemoved);
    connect(model, &QAbstractItemModel::rowsRemoved, this,
            &SubtreeModel::sourceRowsRemoved);
  }

  resetInternalData();
  endResetModel();
}

void SubtreeModel::setRootIndex(const QModelIndex &index) {
  if (rootIndex_ == index) {
    return;
  }

  rootIndex_ = index;
  emit rootIndexChanged();
}

QModelIndex SubtreeModel::mapFromSource(const QModelIndex &sourceIndex) const {
  if (sourceModel() == nullptr || !sourceIndex.isValid() ||
      sourceIndex.parent() != rootIndex_) {
    return QModelIndex();
  }

  return createIndex(sourceIndex.row(), sourceIndex.column(),
                     sourceIndex.internalPointer());
}

QModelIndex SubtreeModel::mapToSource(const QModelIndex &proxyIndex) const {
  if (!proxyIndex.isValid()) {
    return rootIndex_;
  }

  return rootIndex_.child(proxyIndex.row(), proxyIndex.column());
}

QModelIndex SubtreeModel::index(int row, int column,
                              const QModelIndex &parent) const {
  if (sourceModel() == nullptr) {
    return QModelIndex();
  }

  auto sourceParent = mapToSource(parent);
  auto sourceIndex = sourceModel()->index(row, column, sourceParent);
  return mapFromSource(sourceIndex);
}

QModelIndex SubtreeModel::parent(const QModelIndex &child) const {
  if (sourceModel() == nullptr) {
    return QModelIndex();
  }

  auto sourceIndex = mapToSource(child);
  auto sourceParent = sourceModel()->parent(sourceIndex);
  return mapFromSource(sourceParent);
}

int SubtreeModel::rowCount(const QModelIndex &parent) const {
  return sourceModel()->rowCount(mapToSource(parent));
}

int SubtreeModel::columnCount(const QModelIndex &parent) const {
  return sourceModel()->columnCount(mapToSource(parent));
}

void SubtreeModel::sourceDataChanged(const QModelIndex &topLeft,
                                   const QModelIndex &bottomRight,
                                   const QVector<int> &roles) {
  QModelIndex targetTopLeft = mapFromSource(topLeft);
  QModelIndex targetBottomRight = mapFromSource(bottomRight);

  if (!topLeft.isValid() || !bottomRight.isValid()) {
    return;
  }

  emit dataChanged(targetTopLeft, targetBottomRight, roles);
}

void SubtreeModel::sourceRowsAboutToBeInserted(const QModelIndex &sourceParent,
                                             int start, int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  beginInsertRows(QModelIndex(), start, end);
}

void SubtreeModel::sourceRowsInserted(const QModelIndex &sourceParent, int start,
                                    int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  endInsertRows();
}

void SubtreeModel::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent,
                                            int start, int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  beginRemoveRows(QModelIndex(), start, end);
}

void SubtreeModel::sourceRowsRemoved(const QModelIndex &sourceParent, int start,
                                   int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  endRemoveRows();
}

QModelIndex SubtreeModel::modelIndexForSelection(int index) {
  return mapToSource(this->index(index, 0, QModelIndex()));
}

}  // namespace chaoskit::ui
