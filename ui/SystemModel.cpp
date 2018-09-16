#include "SystemModel.h"

using chaoskit::core::Blend;

namespace chaoskit {
namespace ui {

namespace {

enum RowType {
  UNDEFINED = 0,
  BLEND = 1,
  FORMULA = 2,
  PRE_TRANSFORM = 3,
  POST_TRANSFORM = 4,
};

const uint64_t FINAL_BLEND_INDEX = 0x00000fffffffffff;

constexpr quintptr makeIdForBlend(uint64_t blendIndex) {
  return blendIndex << 20;
}
constexpr quintptr makeIdForPre(uint64_t blendIndex) {
  return blendIndex << 20 | 0x10000;
}
constexpr quintptr makeIdForPost(uint64_t blendIndex) {
  return blendIndex << 20 | 0x20000;
}
constexpr quintptr makeIdForFormula(uint64_t blendIndex, int formulaIndex) {
  return blendIndex << 20 | 0x30000 | (formulaIndex & 0xffff);
}

constexpr RowType rowTypeForId(quintptr id) {
  int typeNumber = static_cast<int>((id & 0xf0000) >> 16);

  switch (typeNumber) {
    case 0:
      return BLEND;
    case 1:
      return PRE_TRANSFORM;
    case 2:
      return POST_TRANSFORM;
    case 3:
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

  if (row == 0) {
    return createIndex(row, column, makeIdForPre(blendIndex));
  }

  const auto &parentBlend = (blendIndex == FINAL_BLEND_INDEX)
                                ? system_.final_blend
                                : system_.blends.at(blendIndex);
  auto formulaCount = parentBlend.formulas.size();

  if (row == formulaCount + 1) {
    return createIndex(row, column, makeIdForPost(blendIndex));
  } else if (row <= formulaCount) {
    return createIndex(row, column, makeIdForFormula(blendIndex, row - 1));
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

    // Extra 2 are for the transforms.
    return static_cast<int>(parentBlend.formulas.size()) + 2;
  }

  return 0;
}

int SystemModel::columnCount(const QModelIndex &parent) const { return 1; }

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
      if (blendIndex == FINAL_BLEND_INDEX) {
        return finalBlendData(system_.final_blend, index.column(), role);
      }
      return blendData(system_.blends[blendIndex], index.column(), role);
    }
    case RowType::FORMULA:
      return formulaData(blend.formulas[formulaIndexForId(id)], index.column(),
                         role);
    case RowType::PRE_TRANSFORM:
      return preTransformData(blend.pre, index.column(), role);
    case RowType::POST_TRANSFORM:
      return postTransformData(blend.post, index.column(), role);
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

QVariant SystemModel::blendData(const core::Blend &blend, int column,
                                int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        return QStringLiteral("Blend");  // TODO: change
      default:;
    }
  }

  return QVariant();
}
QVariant SystemModel::finalBlendData(const core::Blend &blend, int column,
                                     int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        return QStringLiteral("Final Blend");
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
      default:;
    }
  }

  return QVariant();
}
QVariant SystemModel::preTransformData(const core::Transform &transform,
                                       int column, int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        return QStringLiteral("Pre Transform");
      default:;
    }
  }

  return QVariant();
}
QVariant SystemModel::postTransformData(const core::Transform &transform,
                                        int column, int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        return QStringLiteral("Post Transform");
      default:;
    }
  }

  return QVariant();
}

}  // namespace ui
}  // namespace chaoskit
