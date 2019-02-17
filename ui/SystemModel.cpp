#include "SystemModel.h"
#include <QDebug>
#include <QTransform>

using chaoskit::ui::models::System;

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
  auto *formula = new Formula();
  formula->setType(QStringLiteral("DeJong"));
  formula->setParams({9.379666578024626e-01f, 1.938709271140397e+00f,
                      -1.580897020176053e-01f, -1.430070123635232e+00f});

  auto *blend = new Blend();
  blend->addFormula(formula);

  auto *finalBlend = new Blend();
  finalBlend->setPost(QTransform::fromScale(.5, 1).translate(.5, .5));

  system_ = new System(this);
  system_->addBlend(blend);
  system_->setFinalBlend(finalBlend);
}

const Blend *SystemModel::getBlendForId(uint64_t id) const {
  auto index = blendIndexForId(id);
  return index == FINAL_BLEND_INDEX ? system_->finalBlend()
                                    : system_->blends()[index];
}

QModelIndex SystemModel::index(int row, int column,
                               const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  // Root level
  if (!parent.isValid()) {
    if (row < system_->blends().size()) {
      return createIndex(row, column, makeIdForBlend(static_cast<size_t>(row)));
    } else if (row == system_->blends().size()) {
      return createIndex(row, column, makeIdForBlend(FINAL_BLEND_INDEX));
    } else {
      return QModelIndex();
    }
  }

  // Blend level
  auto parentId = parent.internalId();
  auto formulaCount = getBlendForId(parentId)->formulas().size();

  if (row < formulaCount) {
    return createIndex(row, column,
                       makeIdForFormula(blendIndexForId(parentId), row));
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
    return createIndex(system_->blends().size(), 0,
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
    return system_->blends().size() + 1;
  }

  // Blend level
  auto parentId = parent.internalId();
  if (rowTypeForId(parentId) == RowType::BLEND) {
    return getBlendForId(parentId)->formulas().size();
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
  const auto *blend = getBlendForId(id);

  switch (rowTypeForId(id)) {
    case RowType::BLEND: {
      auto isFinalBlend = (blendIndexForId(id) == FINAL_BLEND_INDEX);

      return blendData(blend, isFinalBlend, index.column(), role);
    }
    case RowType::FORMULA:
      return formulaData(blend->formulas()[formulaIndexForId(id)],
                         index.column(), role);
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

QVariant SystemModel::blendData(const Blend *blend, bool isFinal, int column,
                                int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        if (isFinal) {
          return QStringLiteral("Final Blend");
        }
        if (blend->name().isEmpty()) {
          return QStringLiteral("(unnamed)");
        }
        return blend->name();
      case WeightRole:
        return blend->weight();
      default:;
    }
  }

  return QVariant();
}

QVariant SystemModel::formulaData(const Formula *formula, int column,
                                  int role) const {
  if (column == 0) {
    switch (role) {
      case Qt::DisplayRole:
        return QString(formula->type()._to_string());
      case WeightRole:
        return formula->weightX();  // TODO: support weight_y
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
