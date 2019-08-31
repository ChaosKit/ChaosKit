#include "DocumentModel.h"
#include <QtGui/QTransform>
#include "state/Id.h"

using chaoskit::state::Id;

namespace chaoskit::ui {

namespace {
quintptr fromId(Id id) { return static_cast<uint64_t>(id); }

Id toId(quintptr ptr) { return Id(static_cast<uint64_t>(ptr)); }
}  // namespace

DocumentModel::DocumentModel(QObject* parent) : QAbstractItemModel(parent) {
  fixInvariants();
}

///////////////////////////////////////////////////////////////////// Custom API

bool DocumentModel::isBlend(const QModelIndex& index) const {
  return matchesType<core::Blend>(index) ||
         matchesType<core::FinalBlend>(index);
}

QModelIndex DocumentModel::blendAt(int i) const {
  return index(i, 0, systemIndex());
}

QModelIndex DocumentModel::formulaAt(int i,
                                     const QModelIndex& blendIndex) const {
  if (!blendIndex.isValid() || !isBlend(blendIndex)) {
    return QModelIndex();
  }

  return index(i, 0, blendIndex);
}

QModelIndex DocumentModel::documentIndex() const {
  return createIndex(0, 0, fromId(documentId()));
}

QModelIndex DocumentModel::systemIndex() const {
  return createIndex(0, 0, fromId(systemId()));
}

QModelIndex DocumentModel::finalBlendIndex() const {
  auto system = systemIndex();
  return index(rowCount(system) - 1, 0, system);
}

const core::Document* DocumentModel::document() const {
  return store_.find<core::Document>(documentId());
}

///////////////////////////////////////////////////////////// Custom API — Slots

QModelIndex DocumentModel::addBlend() {
  size_t newRowNumber = store_.countChildren<core::Blend>(systemId());
  beginInsertRows(systemIndex(), newRowNumber, newRowNumber);
  store_.associateNewChildWith<core::System, core::Blend>(
      systemId(), [](core::Blend* blend) { blend->name = "Unnamed blend"; });
  endInsertRows();
  return blendAt(newRowNumber);
}

QModelIndex DocumentModel::addFormula(library::FormulaType type,
                                      const QModelIndex& blendIndex) {
  if (!blendIndex.isValid() || !isBlend(blendIndex)) {
    return QModelIndex();
  }

  Id blendId = toId(blendIndex.internalId());
  size_t newRowNumber = store_.countChildren<core::Formula>(blendId);

  beginInsertRows(blendIndex, newRowNumber, newRowNumber);

  Id formulaId = store_.create<core::Formula>(
      [type](core::Formula* formula) { formula->setType(type); });

  if (Store::matchesType<core::FinalBlend>(blendId)) {
    store_.associateChildWith<core::FinalBlend, core::Formula>(blendId,
                                                               formulaId);
  } else {
    store_.associateChildWith<core::Blend, core::Formula>(blendId, formulaId);
  }

  endInsertRows();
  return index(newRowNumber, 0, blendIndex);
}

/////////////////////// QAbstractItemModel method overrides for read-only access

Qt::ItemFlags DocumentModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (Store::matchesType<core::Blend>(toId(index.internalId()))) {
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

QModelIndex DocumentModel::index(int row, int column,
                                 const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  // The root is the Document.
  if (!parent.isValid()) {
    return documentIndex();
  }

  Id parentId = toId(parent.internalId());

  // Special case for children of the System. Blends and the Final Blend should
  // be displayed in that order.
  if (Store::matchesType<core::System>(parentId)) {
    size_t offset = 0;
    const auto& blends = store_.children<core::Blend>(parentId);
    offset = blends.size();
    if (row < blends.size()) {
      return createIndex(row, column, fromId(blends[row]));
    }

    const auto& finalBlends = store_.children<core::FinalBlend>(parentId);
    if (row < finalBlends.size() + offset) {
      return createIndex(row, column, fromId(finalBlends[row - offset]));
    }

    return QModelIndex();
  }

  // Generic tree traversal for children.
  const auto& children = store_.allChildren(parentId);
  if (row < children.size()) {
    return createIndex(row, column, fromId(children[row]));
  }

  return QModelIndex();
}

QModelIndex DocumentModel::parent(const QModelIndex& child) const {
  if (!child.isValid()) {
    return QModelIndex();
  }

  Id childId = toId(child.internalId());
  const Id* parentId = store_.parent(childId);

  if (!parentId) {
    // Child is the Document.
    return QModelIndex();
  }

  const Id* grandparentId = store_.parent(*parentId);
  if (!grandparentId) {
    // Child is the System, so the parent is the Document.
    return documentIndex();
  }

  // Special case for Blends and the Final Blend because of their forced
  // ordering.
  if (Store::matchesType<core::Blend>(*parentId)) {
    const auto& blends = store_.children<core::Blend>(*grandparentId);
    auto it = std::find(blends.begin(), blends.end(), *parentId);
    return createIndex(std::distance(blends.begin(), it), 0, fromId(*parentId));
  }
  if (Store::matchesType<core::FinalBlend>(*parentId)) {
    size_t blendCount = store_.countChildren<core::Blend>(*grandparentId);
    return createIndex(blendCount, 0, fromId(*parentId));
  }

  // Generic case for other parents.
  const auto& parents = store_.allChildren(*grandparentId);
  auto it = std::find(parents.begin(), parents.end(), *parentId);
  return createIndex(std::distance(parents.begin(), it), 0, fromId(*parentId));
}

int DocumentModel::rowCount(const QModelIndex& parent) const {
  if (!parent.isValid()) {
    // There's just the Document.
    return 1;
  }
  Id parentId = toId(parent.internalId());
  return store_.countAllChildren(parentId);
}

int DocumentModel::columnCount(const QModelIndex& parent) const { return 1; }

bool DocumentModel::hasChildren(const QModelIndex& parent) const {
  if (!parent.isValid()) {
    // The Document is a child.
    return true;
  }

  Id parentId = toId(parent.internalId());
  return store_.hasAnyChildren(parentId);
}

/////////////////////////////////////////////////////// Read-only data functions

namespace {

QTransform toQtTransform(const core::Transform& transform) {
  return QTransform(transform.values[0], transform.values[3],
                    transform.values[1], transform.values[4],
                    transform.values[2], transform.values[5]);
}

core::Transform fromQtTransform(const QTransform& transform) {
  return core::Transform(
      {static_cast<float>(transform.m11()), static_cast<float>(transform.m21()),
       static_cast<float>(transform.m31()), static_cast<float>(transform.m12()),
       static_cast<float>(transform.m22()),
       static_cast<float>(transform.m32())});
}

QVariant documentData(const core::Document* blend, int role) {
  if (!blend) return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return QStringLiteral("Document");
    default:
      return QVariant();
  }
}
QVariant systemData(const core::System* blend, int role) {
  if (!blend) return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return QStringLiteral("System");
    default:
      return QVariant();
  }
}
QVariant commonBlendData(const core::BlendBase* blend, int role) {
  switch (role) {
    case DocumentModel::PreTransformRole:
      return QVariant::fromValue(toQtTransform(blend->pre));
    case DocumentModel::PostTransformRole:
      return QVariant::fromValue(toQtTransform(blend->post));
    default:
      return QVariant();
  }
}
QVariant blendData(const core::Blend* blend, int role) {
  if (!blend) return QVariant();

  switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return QString::fromStdString(blend->name);
    default:
      return commonBlendData(blend, role);
  }
}
QVariant finalBlendData(const core::FinalBlend* blend, int role) {
  if (!blend) return QVariant();

  if (role == Qt::DisplayRole) {
    return QStringLiteral("Final Blend");
  }

  return commonBlendData(blend, role);
}
QVariant formulaData(const core::Formula* formula, int role) {
  if (!formula) return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return QString(formula->type._to_string());
    case DocumentModel::ParamsRole:
      return QVariant::fromValue(formula->params);
  }

  return QVariant();
}

bool setCommonBlendData(core::BlendBase* blend, const QVariant& value,
                        int role) {
  switch (role) {
    case DocumentModel::PreTransformRole:
      blend->pre = fromQtTransform(value.value<QTransform>());
      return true;
    case DocumentModel::PostTransformRole:
      blend->post = fromQtTransform(value.value<QTransform>());
      return true;
    default:
      return false;
  }
}
bool setBlendData(core::Blend* blend, const QVariant& value, int role) {
  if (!blend) return false;

  if (role == Qt::EditRole) {
    blend->name = value.toString().toStdString();
    return true;
  }

  return setCommonBlendData(blend, value, role);
}
bool setFinalBlendData(core::FinalBlend* blend, const QVariant& value,
                       int role) {
  if (!blend) return false;

  return setCommonBlendData(blend, value, role);
}
bool setFormulaData(core::Formula* formula, const QVariant& value, int role) {
  if (!formula) return false;

  if (role == DocumentModel::ParamsRole) {
    formula->params = value.value<std::vector<float>>();
    return true;
  }

  return false;
}

}  // namespace

QVariant DocumentModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  Id id = toId(index.internalId());
  if (Store::matchesType<core::Document>(id)) {
    return documentData(store_.find<core::Document>(id), role);
  }
  if (Store::matchesType<core::System>(id)) {
    return systemData(store_.find<core::System>(id), role);
  }
  if (Store::matchesType<core::Blend>(id)) {
    return blendData(store_.find<core::Blend>(id), role);
  }
  if (Store::matchesType<core::FinalBlend>(id)) {
    return finalBlendData(store_.find<core::FinalBlend>(id), role);
  }
  if (Store::matchesType<core::Formula>(id)) {
    return formulaData(store_.find<core::Formula>(id), role);
  }
  return QVariant();
}

QVariant DocumentModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    return QStringLiteral("Name");
  }

  return QVariant();
}

///////////////////////////////////////////////////////// Write access overrides

bool DocumentModel::setData(const QModelIndex& index, const QVariant& value,
                            int role) {
  if (!index.isValid()) {
    return false;
  }

  Id id = toId(index.internalId());
  bool updated = false;
  if (Store::matchesType<core::Blend>(id)) {
    updated =
        store_.update<core::Blend>(id, [&value, role](core::Blend* blend) {
          return setBlendData(blend, value, role);
        });
  } else if (Store::matchesType<core::FinalBlend>(id)) {
    updated = store_.update<core::FinalBlend>(
        id, [&value, role](core::FinalBlend* blend) {
          return setFinalBlendData(blend, value, role);
        });
  } else if (Store::matchesType<core::Formula>(id)) {
    updated = store_.update<core::Formula>(
        id, [&value, role](core::Formula* formula) {
          return setFormulaData(formula, value, role);
        });
  }

  if (updated) {
    emit dataChanged(index, index, {role});
  }
  return updated;
}

bool DocumentModel::removeRows(int row, int count, const QModelIndex& parent) {
  if (count < 1) {
    return false;
  }

  if (!parent.isValid()) {
    // Trying to remove the Document, remove the System instead.
    return removeRows(0, 1, documentIndex());
  }

  Id parentId = toId(parent.internalId());
  if (Store::matchesType<core::Document>(parentId)) {
    // Trying to remove the System, remove all Blends.
    auto system = systemIndex();
    return removeRows(0, rowCount(system), system);
  }

  const std::vector<Id>* children;
  if (Store::matchesType<core::System>(parentId)) {
    // Trying to remove Blends

    if (row + count > store_.countChildren<core::Blend>(parentId)) {
      // Trying to remove the Final Blend, split the range to delete Blends and
      // Final Blend's Formulas separately.
      auto finalBlend = index(rowCount(parent) - 1, 0, parent);

      removeRows(0, rowCount(finalBlend), finalBlend);
      removeRows(row, count - 1, parent);
      return false;
    }

    children = &store_.children<core::Blend>(parentId);
  } else {
    children = &store_.allChildren(parentId);
  }

  std::vector<Id> idsToRemove;
  if (children) {
    auto front = children->begin() + row;
    auto back = front + count;
    if (front != children->end()) {
      std::copy(front, back, std::back_inserter(idsToRemove));
    }
  }

  if (idsToRemove.size() != count) {
    return false;
  }

  beginRemoveRows(parent, row, row + count);
  for (auto id : idsToRemove) {
    store_.remove(id);
  }
  endRemoveRows();

  return true;
}

//////////////////////////////////////////////////////////////// Private methods

state::Id DocumentModel::documentId() const {
  return store_.lastId<core::Document>();
};

state::Id DocumentModel::systemId() const {
  return store_.lastId<core::System>();
};

void DocumentModel::fixInvariants() {
  Id documentId = (store_.count<core::Document>() < 1)
                      ? store_.create<core::Document>()
                      : store_.lastId<core::Document>();
  Id systemId =
      (store_.countChildren<core::System>(documentId) < 1)
          ? store_.associateNewChildWith<core::Document, core::System>(
                documentId)
          : store_.lastId<core::System>();

  if (store_.countChildren<core::FinalBlend>(systemId) < 1) {
    store_.associateNewChildWith<core::System, core::FinalBlend>(systemId);
  }
}

}  // namespace chaoskit::ui
