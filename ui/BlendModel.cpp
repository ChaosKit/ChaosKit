#include "BlendModel.h"

namespace chaoskit::ui {

void BlendModel::setSourceModel(QAbstractItemModel *model) {
  beginResetModel();

  if (sourceModel()) {
    sourceModel()->disconnect(this);
  }

  QAbstractProxyModel::setSourceModel(model);

  if (model) {
    connect(model, &QAbstractItemModel::dataChanged, this,
            &BlendModel::sourceDataChanged);
    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this,
            &BlendModel::sourceRowsAboutToBeInserted);
    connect(model, &QAbstractItemModel::rowsInserted, this,
            &BlendModel::sourceRowsInserted);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            &BlendModel::sourceRowsAboutToBeRemoved);
    connect(model, &QAbstractItemModel::rowsRemoved, this,
            &BlendModel::sourceRowsRemoved);
  }

  resetInternalData();
  endResetModel();
}

void BlendModel::setRootIndex(const QModelIndex &index) {
  if (rootIndex_ == index) {
    return;
  }

  rootIndex_ = index;
  emit rootIndexChanged();
}

QModelIndex BlendModel::mapFromSource(const QModelIndex &sourceIndex) const {
  if (sourceModel() == nullptr || !sourceIndex.isValid() ||
      sourceIndex.parent() != rootIndex_) {
    return QModelIndex();
  }

  return createIndex(sourceIndex.row(), sourceIndex.column(),
                     sourceIndex.internalPointer());
}

QModelIndex BlendModel::mapToSource(const QModelIndex &proxyIndex) const {
  if (!proxyIndex.isValid()) {
    return rootIndex_;
  }

  return rootIndex_.child(proxyIndex.row(), proxyIndex.column());
}

QModelIndex BlendModel::index(int row, int column,
                              const QModelIndex &parent) const {
  if (sourceModel() == nullptr) {
    return QModelIndex();
  }

  auto sourceParent = mapToSource(parent);
  auto sourceIndex = sourceModel()->index(row, column, sourceParent);
  return mapFromSource(sourceIndex);
}

QModelIndex BlendModel::parent(const QModelIndex &child) const {
  if (sourceModel() == nullptr) {
    return QModelIndex();
  }

  auto sourceIndex = mapToSource(child);
  auto sourceParent = sourceModel()->parent(sourceIndex);
  return mapFromSource(sourceParent);
}

int BlendModel::rowCount(const QModelIndex &parent) const {
  return sourceModel()->rowCount(mapToSource(parent));
}

int BlendModel::columnCount(const QModelIndex &parent) const {
  return sourceModel()->columnCount(mapToSource(parent));
}

void BlendModel::sourceDataChanged(const QModelIndex &topLeft,
                                   const QModelIndex &bottomRight,
                                   const QVector<int> &roles) {
  QModelIndex targetTopLeft = mapFromSource(topLeft);
  QModelIndex targetBottomRight = mapFromSource(bottomRight);

  if (!topLeft.isValid() || !bottomRight.isValid()) {
    return;
  }

  emit dataChanged(targetTopLeft, targetBottomRight, roles);
}

void BlendModel::sourceRowsAboutToBeInserted(const QModelIndex &sourceParent,
                                             int start, int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  beginInsertRows(QModelIndex(), start, end);
}

void BlendModel::sourceRowsInserted(const QModelIndex &sourceParent, int start,
                                    int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  endInsertRows();
}

void BlendModel::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent,
                                            int start, int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  beginRemoveRows(QModelIndex(), start, end);
}

void BlendModel::sourceRowsRemoved(const QModelIndex &sourceParent, int start,
                                   int end) {
  if (!sourceParent.isValid() || sourceParent != rootIndex_) {
    return;
  }

  endRemoveRows();
}

QModelIndex BlendModel::modelIndexForSelection(int index) {
  return mapToSource(this->index(index, 0, QModelIndex()));
}

}  // namespace chaoskit::ui
