#include "FlatteningModel.h"

namespace chaoskit {
namespace ui {

void FlatteningModel::setRootIndex(const QModelIndex &index) {
  if (rootIndex_ == index) {
    return;
  }

  rootIndex_ = index;
  emit rootIndexChanged();
}

QModelIndex FlatteningModel::mapFromSource(
    const QModelIndex &sourceIndex) const {
  if (sourceModel() == nullptr || !sourceIndex.isValid() || sourceIndex.parent() != rootIndex_) {
    return QModelIndex();
  }

  return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
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

}  // namespace ui
}  // namespace chaoskit
