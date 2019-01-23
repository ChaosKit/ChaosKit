#include "SystemModel.h"

using chaoskit::core::Blend;

namespace chaoskit {
namespace ui {

namespace {

enum RowType {
  UNDEFINED = 0,
  BLEND = 1,
  FORMULA = 2,
};

const uint64_t FINAL_BLEND_INDEX = 0x00000fffffffffff;

constexpr quintptr makeIdForBlend(uint64_t blendIndex) {
  return blendIndex << 20;
}
constexpr quintptr makeIdForFormula(uint64_t blendIndex, int formulaIndex) {
  return blendIndex << 20 | 0x10000 | (formulaIndex & 0xffff);
}

constexpr RowType rowTypeForId(quintptr id) {
  int typeNumber = static_cast<int>((id & 0xf0000) >> 16);

  switch (typeNumber) {
    case 0:
      return BLEND;
    case 1:
      return FORMULA;
    default:
      return UNDEFINED;
  }
}

constexpr uint64_t blendIndexForId(quintptr id) { return id >> 20; }

constexpr int formulaIndexForId(quintptr id) {
  return static_cast<int>(id & 0xffff);
}

}  // namespace

SystemModel::SystemModel(QObject *parent) : QAbstractItemModel(parent) {
  // TODO: replace this with something else maybe
  system_ = System().system();
}

QModelIndex SystemModel::index(int row, int column,
                               const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  // Root level
  if (!parent.isValid()) {
    if (row < system_.blends.size()) {
      return createIndex(row, column, makeIdForBlend(static_cast<size_t>(row)));
    } else if (row == system_.blends.size()) {
      return createIndex(row, column, makeIdForBlend(FINAL_BLEND_INDEX));
    } else {
      return QModelIndex();
    }
  }

  // Blend level
  auto parentId = parent.internalId();
  auto blendIndex = blendIndexForId(parentId);

  const auto &parentBlend = (blendIndex == FINAL_BLEND_INDEX)
                                ? system_.final_blend
                                : system_.blends.at(blendIndex);
  auto formulaCount = parentBlend.formulas.size();

  if (row < formulaCount) {
    return createIndex(row, column, makeIdForFormula(blendIndex, row));
  }

  return QModelIndex();
}

QModelIndex SystemModel::parent(const QModelIndex &child) const {
  if (!child.isValid()) {
    return QModelIndex();
  }

  auto childId = child.internalId();
  if (rowTypeForId(childId) == RowType::BLEND) {
    return QModelIndex();
  }

  uint64_t blendIndex = blendIndexForId(childId);
  if (blendIndex == FINAL_BLEND_INDEX) {
    return createIndex(static_cast<int>(system_.blends.size()), 0,
                       makeIdForBlend(FINAL_BLEND_INDEX));
  }
  return createIndex(static_cast<int>(blendIndex), 0,
                     makeIdForBlend(blendIndex));
}

int SystemModel::rowCount(const QModelIndex &parent) const {
  if (parent.column() > 0) {
    return 0;
  }

  // Root level
  if (!parent.isValid()) {
    // Extra one is for the final blend.
    return static_cast<int>(system_.blends.size()) + 1;
  }

  // Blend level
  auto parentId = parent.internalId();
  if (rowTypeForId(parentId) == RowType::BLEND) {
    uint64_t blendIndex = blendIndexForId(parentId);
    const auto &parentBlend = (blendIndex == FINAL_BLEND_INDEX)
                                  ? system_.final_blend
                                  : system_.blends[blendIndex];

    return static_cast<int>(parentBlend.formulas.size());
  }

  return 0;
}

int SystemModel::columnCount(const QModelIndex &parent) const { return 1; }

QHash<int, QByteArray> SystemModel::roleNames() const {
  QHash<int, QByteArray> names;
  names[Qt::DisplayRole] = "display";
  names[WeightRole] = "weight";
  return names;
}

QVariant SystemModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  auto id = index.internalId();
  uint64_t blendIndex = blendIndexForId(id);
  const auto &blend = (blendIndex == FINAL_BLEND_INDEX)
                          ? system_.final_blend
                          : system_.blends[blendIndex];

  switch (rowTypeForId(id)) {
    case RowType::BLEND: {
      auto isFinalBlend = (blendIndex == FINAL_BLEND_INDEX);

      return blendData(
          isFinalBlend ? system_.final_blend : system_.blends[blendIndex],
          isFinalBlend, index.column(), role);
    }
    case RowType::FORMULA:
      return formulaData(blend.formulas[formulaIndexForId(id)], index.column(),
                         role);
    case RowType::UNDEFINED:
      return QVariant();
  }
  return QVariant();
}

QVariant SystemModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const {
  return QVariant();
}

Qt::ItemFlags SystemModel::flags(const QModelIndex &index) const {
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
QVariant SystemModel::blendData(const core::Blend &blend, bool isFinal,
                                int column, int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        // TODO: change this
        return isFinal ? QStringLiteral("Final Blend")
                       : QStringLiteral("Blend");
      case WeightRole:
        return blend.weight;
      default:;
    }
  }

  return QVariant();
}

QVariant SystemModel::formulaData(const core::Formula &formula, int column,
                                  int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        return QStringLiteral("Formula");  // TODO: change
      case WeightRole:
        return formula.weight_x;  // TODO: support weight_y
      default:;
    }
  }

  return QVariant();
}
FlatteningModel *SystemModel::childModel(int index) {
  auto *model = new FlatteningModel();
  model->setSourceModel(this);
  model->setRootIndex(this->index(index, 0, QModelIndex()));
  return model;
}

}  // namespace ui
}  // namespace chaoskit
