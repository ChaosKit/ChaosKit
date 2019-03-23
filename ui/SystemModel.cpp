#include "SystemModel.h"
#include <ast/ast.h>
#include <library/FormulaType.h>
#include <QDebug>
#include <QTransform>
#include <sstream>
#include "models/toSource.h"

using chaoskit::ui::System;
using chaoskit::library::FormulaType;

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
  system_ = new System(this);
  auto *formula = system_->addBlend()->addFormula(FormulaType::DeJong);
  formula->setParams({9.379666578024626e-01f, 1.938709271140397e+00f,
                      -1.580897020176053e-01f, -1.430070123635232e+00f});
  system_->finalBlend()->setPost(
      QTransform::fromScale(.5, 1).translate(.5, .5));

  connect(system_, &System::sourceChanged, this, &SystemModel::sourceChanged);
  connect(system_, &System::finalBlendSourceChanged, this, &SystemModel::sourceChanged);
}

QString SystemModel::source() const {
  auto sourceAst = toSource(system_);
  std::stringstream stream;
  stream << sourceAst;
  return QString::fromStdString(stream.str());
}

Blend *SystemModel::getBlendForId(uint64_t id) const {
  auto index = blendIndexForId(id);
  if (index == FINAL_BLEND_INDEX) {
    return system_->finalBlend();
  }
  return system_->blendAt(static_cast<int>(index));
}

QModelIndex SystemModel::index(int row, int column,
                               const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  // Root level
  if (!parent.isValid()) {
    if (row < system_->blendCount()) {
      return createIndex(row, column, makeIdForBlend(static_cast<size_t>(row)));
    } else if (row == system_->blendCount()) {
      return createIndex(row, column, makeIdForBlend(FINAL_BLEND_INDEX));
    } else {
      return QModelIndex();
    }
  }

  // Blend level
  auto parentId = parent.internalId();
  auto formulaCount = getBlendForId(parentId)->formulaCount();

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
    return createIndex(system_->blendCount(), 0,
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
    return system_->blendCount() + 1;
  }

  // Blend level
  auto parentId = parent.internalId();
  if (rowTypeForId(parentId) == RowType::BLEND) {
    return getBlendForId(parentId)->formulaCount();
  }

  return 0;
}

int SystemModel::columnCount(const QModelIndex &parent) const { return 1; }

QHash<int, QByteArray> SystemModel::roleNames() const {
  QHash<int, QByteArray> names = QAbstractItemModel::roleNames();
  names[WeightRole] = "weight";
  names[BlendIndexRole] = "blendIndex";
  names[IsFinalBlendRole] = "isFinalBlend";
  return names;
}

QVariant SystemModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.column() != 0) {
    return QVariant();
  }

  auto id = index.internalId();
  const auto *blend = getBlendForId(id);

  // Return the blend index for all types.
  if (role == BlendIndexRole) {
    auto blendIndex = blendIndexForId(id);
    return blendIndex == FINAL_BLEND_INDEX ? system_->blendCount()
                                           : static_cast<int>(blendIndex);
  }

  switch (rowTypeForId(id)) {
    case RowType::BLEND:
      return blendData(blend, role);
    case RowType::FORMULA:
      return formulaData(blend->formulaAt(formulaIndexForId(id)), role);
    case RowType::UNDEFINED:
      return QVariant();
  }
  return QVariant();
}

bool SystemModel::setData(const QModelIndex &index, const QVariant &value,
                          int role) {
  if (!index.isValid() || index.column() != 0) {
    return false;
  }

  auto id = index.internalId();
  auto *blend = getBlendForId(id);
  bool success = false;

  switch (rowTypeForId(id)) {
    case RowType::BLEND:
      success = setBlendData(blend, role, value);
      break;
    case RowType::FORMULA:
      success =
          setFormulaData(blend->formulaAt(formulaIndexForId(id)), role, value);
      break;
    case RowType::UNDEFINED:
      return false;
  }

  if (success) {
    emit dataChanged(index, index, {role});
  }

  return success;
}

bool SystemModel::insertRows(int row, int count, const QModelIndex &parent) {
  // Adding blends
  if (!parent.isValid()) {
    // We can only append.
    if (row != system_->blendCount()) {
      return false;
    }

    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
      system_->addBlend();
    }
    endInsertRows();
    return true;
  }

  // Adding formulas
  if (rowTypeForId(parent.internalId()) == RowType::BLEND) {
    auto *blend = getBlendForId(parent.internalId());

    // We can only append.
    if (row != blend->formulaCount()) {
      return false;
    }

    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
      blend->addFormula(formulaTypeForAdding);
    }
    endInsertRows();
    return true;
  }

  return false;
}

bool SystemModel::removeRows(int row, int count, const QModelIndex &parent) {
  // Removing blends
  if (!parent.isValid()) {
    // Disallow removing the final blend
    if (row == system_->blendCount()) {
      return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
      system_->removeBlendAt(row);
    }
    endRemoveRows();
    return true;
  }

  // Removing formulas
  if (rowTypeForId(parent.internalId()) == RowType::BLEND) {
    auto *blend = getBlendForId(parent.internalId());

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
      blend->removeFormulaAt(row);
    }
    endRemoveRows();
    return true;
  }

  return false;
}

QVariant SystemModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const {
  return QVariant();
}

bool SystemModel::setHeaderData(int section, Qt::Orientation orientation,
                                const QVariant &value, int role) {
  return false;
}

Qt::ItemFlags SystemModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return nullptr;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant SystemModel::blendData(const Blend *blend, int role) const {
  switch (role) {
    case Qt::DisplayRole:
      if (blend == system_->finalBlend()) {
        return QStringLiteral("Final Blend");
      }
      if (blend->name().isEmpty()) {
        return QStringLiteral("(unnamed)");
      }
    case Qt::EditRole:
      return blend->name();
    case WeightRole:
      return blend->weight();
    case IsFinalBlendRole:
      return blend == system_->finalBlend();
    default:;
  }

  return QVariant();
}

bool SystemModel::setBlendData(Blend *blend, int role,
                               const QVariant &value) const {
  if (blend == system_->finalBlend()) {
    return false;
  }

  switch (role) {
    case Qt::EditRole:
      blend->setName(value.toString());
      return true;
    case WeightRole:
      blend->setWeight(value.toFloat());
      return true;
    default:;
  }

  return false;
}

QVariant SystemModel::formulaData(const Formula *formula, int role) const {
  switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return QString(formula->type()._to_string());
    case WeightRole:
      return formula->weightX();  // TODO: support weight_y
    case IsFinalBlendRole:
      return false;
    default:;
  }

  return QVariant();
}

bool SystemModel::setFormulaData(Formula *formula, int role,
                                 const QVariant &value) const {
  switch (role) {
    case WeightRole: {
      float weight = value.toFloat();
      formula->setWeightX(weight);
      formula->setWeightY(weight);
      return true;
    }
    default:;
  }

  return false;
}

BlendModel *SystemModel::childModel(int index) {
  auto *model = new BlendModel();
  model->setSourceModel(this);
  model->setRootIndex(this->index(index, 0, QModelIndex()));
  return model;
}

void SystemModel::addBlend() {
  int targetRow = system_->blendCount();
  insertRow(targetRow, QModelIndex());
}

void SystemModel::addFormula(int blendIndex, const QString &type) {
  Blend *blend = blendIndex == system_->blendCount()
                     ? system_->finalBlend()
                     : system_->blendAt(blendIndex);
  int lastIndex = blend->formulaCount();

  formulaTypeForAdding = library::FormulaType::_from_string(type.toUtf8());
  insertRow(lastIndex, index(blendIndex, 0, QModelIndex()));
}

void SystemModel::removeRowAtIndex(const QModelIndex &index) {
  removeRow(index.row(), index.parent());
}

bool SystemModel::isFinalBlend(const QModelIndex &index) {
  return index.isValid() && !index.parent().isValid() &&
         index.row() == system_->blendCount();
}
QModelIndex SystemModel::modelIndexForSelection(int index) {
  return this->index(index, 0, QModelIndex());
}

}  // namespace ui
}  // namespace chaoskit
