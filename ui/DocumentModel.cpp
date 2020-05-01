#include "DocumentModel.h"
#include <QDebug>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QRandomGenerator>
#include <QtGui/QTransform>
#include <magic_enum.hpp>
#include "DocumentAdopter.h"
#include "core/ManagedDocument.h"
#include "core/util.h"
#include "io/io.h"
#include "library/util.h"
#include "state/Id.h"

using chaoskit::state::Id;

QDebug operator<<(QDebug debug, Id id) {
  QDebugStateSaver saver(debug);
  debug.nospace().noquote() << hex << "(" << id.type << ":" << id.id << ")";
  return debug;
}

namespace chaoskit::ui {

namespace {

Q_LOGGING_CATEGORY(logUpdate, "DocumentModel.update");

quintptr fromId(Id id) { return static_cast<uint64_t>(id); }

Id toId(quintptr ptr) { return Id(static_cast<uint64_t>(ptr)); }
Id toId(const QModelIndex& index) { return toId(index.internalId()); }

std::vector<float> generateFormulaParams(library::FormulaType type) {
  std::vector<float> params(library::paramCount(type));
  auto* rng = QRandomGenerator::global();
  for (float& param : params) {
    param = static_cast<float>(rng->bounded(4.0) - 2.0);
  }
  return params;
}

std::vector<float> generateColoringMethodParams(
    library::ColoringMethodType type) {
  std::vector<float> params(library::paramCount(type));
  auto* rng = QRandomGenerator::global();
  for (float& param : params) {
    param = static_cast<float>(type == library::ColoringMethodType::Distance
                                   ? rng->bounded(0.4)
                                   : rng->generateDouble());
  }
  return params;
}

QVector<float> generateUniformWeights(int count) {
  if (count <= 0) {
    return QVector<float>();
  }

  QVector<float> cuts(count - 1);
  auto* rng = QRandomGenerator::global();
  for (float& cut : cuts) {
    cut = (float)rng->generateDouble();
  }
  std::sort(cuts.begin(), cuts.end());

  QVector<float> weights(count);
  for (int i = 0; i < count; ++i) {
    float prev = (i == 0) ? 0.f : cuts[i - 1];
    float next = (i == cuts.size()) ? 1.f : cuts[i];

    weights[i] = next - prev;
  }
  return weights;
}

QColor toQtColor(const core::Color& color) {
  return QColor::fromRgbF(color.r, color.g, color.b, color.a);
}

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

}  // namespace

DocumentModel::DocumentModel(QObject* parent)
    : QAbstractItemModel(parent),
      documentProxy_(new DocumentProxy(this)),
      systemProxy_(new SystemProxy(this)) {
  fixInvariants();

  connect(this, &QAbstractItemModel::dataChanged, this,
          &DocumentModel::handleDataChanges);
  connect(this, &QAbstractItemModel::rowsInserted, this,
          &DocumentModel::handleRowInsertion);
  connect(this, &QAbstractItemModel::rowsRemoved, this,
          &DocumentModel::handleRowRemoval);
  connect(this, &QAbstractItemModel::rowsMoved, this,
          &DocumentModel::handleRowMove);
  connect(this, &QAbstractItemModel::modelReset, this,
          &DocumentModel::structureChanged);
  connect(this, &QAbstractItemModel::modelReset, this,
          &DocumentModel::systemReset);
  connect(this, &QAbstractItemModel::modelReset, this,
          &DocumentModel::documentReset);
}

///////////////////////////////////////////////////////////////////// Custom API

bool DocumentModel::isBlend(const QModelIndex& index) const {
  return matchesType<core::Blend>(index) ||
         matchesType<core::FinalBlend>(index);
}

bool DocumentModel::isFinalBlend(const QModelIndex& index) const {
  return matchesType<core::FinalBlend>(index);
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

bool DocumentModel::removeRowAtIndex(const QModelIndex& index) {
  return removeRow(index.row(), index.parent());
}

const core::Document* DocumentModel::document() const {
  return store_.find<core::Document>(documentId());
}

const core::System* DocumentModel::system() const {
  return store_.find<core::System>(systemId());
}

ModelEntry* DocumentModel::entryAtIndex(const QModelIndex& index) {
  return new ModelEntry(this, index);
}

DocumentProxy* DocumentModel::documentProxy() { return documentProxy_; }

SystemProxy* DocumentModel::systemProxy() { return systemProxy_; }

QString DocumentModel::debugSource() const {
  return QString::fromStdString(core::debugString(*system()));
}

void DocumentModel::adoptDocument(core::Document* document) {
  beginResetModel();

  store_.clearAll();
  DocumentAdopter(store_).visit(*document);

  fixInvariants();
  endResetModel();
}

bool DocumentModel::loadFromFile(const QString& path) {
  auto doc = core::makeManagedDocument();

  try {
    io::loadFromFile(path.toStdString(), doc.get());
  } catch (io::Error& e) {
    emit ioFailed(QString::fromUtf8(e.what()));
    return false;
  }

  adoptDocument(doc.get());

  setModified(false);
  setFilePath(path);

  return true;
}

bool DocumentModel::saveToFile(const QString& path) {
  try {
    io::saveToFile(path.toStdString(), *document());
  } catch (io::Error& e) {
    emit ioFailed(QString::fromUtf8(e.what()));
    return false;
  }

  setModified(false);
  setFilePath(path);

  return true;
}

QColor DocumentModel::colorAt(qreal position) {
  const core::ColorMap* colorMap = colorMapRegistry_->get(document()->colorMap);
  if (!colorMap) return QColor();
  return toQtColor(colorMap->map(position));
}

///////////////////////////////////////////////////////////// Custom API — Slots

QModelIndex DocumentModel::addBlend() {
  size_t newRowNumber = store_.countChildren<core::Blend>(systemId());
  beginInsertRows(systemIndex(), newRowNumber, newRowNumber);
  store_.associateNewChildWith<core::System, core::Blend>(systemId());
  endInsertRows();
  return blendAt(newRowNumber);
}

QModelIndex DocumentModel::addBlend(chaoskit::library::FormulaType type) {
  size_t newRowNumber = store_.countChildren<core::Blend>(systemId());
  beginInsertRows(systemIndex(), newRowNumber, newRowNumber);

  Id blendId =
      store_.associateNewChildWith<core::System, core::Blend>(systemId());
  store_.associateNewChildWith<core::Blend, core::Formula>(
      blendId, [type](core::Formula* formula) {
        formula->setType(type);
        formula->params = generateFormulaParams(type);
      });

  endInsertRows();
  return blendAt(newRowNumber);
}

QModelIndex DocumentModel::addBlend(const QString& type) {
  auto optionalType =
      magic_enum::enum_cast<library::FormulaType>(type.toStdString());
  if (!optionalType) {
    return QModelIndex();
  }

  return addBlend(*optionalType);
}

QModelIndex DocumentModel::addFormula(library::FormulaType type,
                                      const QModelIndex& blendIndex) {
  if (!blendIndex.isValid() || !isBlend(blendIndex)) {
    return QModelIndex();
  }

  Id blendId = toId(blendIndex.internalId());
  size_t newRowNumber = store_.countChildren<core::Formula>(blendId);

  beginInsertRows(blendIndex, newRowNumber, newRowNumber);

  Id formulaId = store_.create<core::Formula>([type](core::Formula* formula) {
    formula->setType(type);
    formula->params = generateFormulaParams(type);
  });

  if (DocumentStore::matchesType<core::FinalBlend>(blendId)) {
    store_.associateChildWith<core::FinalBlend, core::Formula>(blendId,
                                                               formulaId);
  } else {
    store_.associateChildWith<core::Blend, core::Formula>(blendId, formulaId);
  }

  endInsertRows();
  return index(newRowNumber, 0, blendIndex);
}

QModelIndex DocumentModel::addFormula(const QString& type,
                                      const QModelIndex& blendIndex) {
  auto optionalType =
      magic_enum::enum_cast<library::FormulaType>(type.toStdString());
  if (!optionalType) {
    return QModelIndex();
  }

  return addFormula(*optionalType, blendIndex);
}

void DocumentModel::randomizeParams(const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }

  // Get the FormulaType
  Id id = toId(index.internalId());
  if (!DocumentStore::matchesType<core::Formula>(id)) {
    return;
  }
  library::FormulaType formulaType = store_.find<core::Formula>(id)->type;

  setData(index, QVariant::fromValue(generateFormulaParams(formulaType)),
          DocumentModel::ParamsRole);
}

void DocumentModel::randomizeSystem() {
  auto settings = RandomizationSettings::Builder()
                      .setMaxBlends(2)
                      .setMaxFormulasInBlend(2)
                      .setAllowedFormulaTypes({library::FormulaType::DeJong,
                                               library::FormulaType::Drain})
                      .setAllowedColoringMethodsInBlend(
                          {library::ColoringMethodType::SingleColor,
                           library::ColoringMethodType::Distance})
                      .setAllowedColoringMethodsInFinalBlend(
                          {library::ColoringMethodType::Noop})
                      .build();

  randomizeSystem(settings);
}

void DocumentModel::randomizeSystem(const RandomizationSettings& settings) {
  beginResetModel();

  // Remove the System and then recreate a blank one
  store_.remove(systemId());
  fixInvariants();

  auto* rng = QRandomGenerator::global();

  int numBlends = rng->bounded(settings.minBlends(), settings.maxBlends() + 1);
  QVector<float> blendWeights = generateUniformWeights(numBlends);

  // Generate blends
  for (int i = 0; i < numBlends; ++i) {
    float blendWeight = blendWeights[i];

    int coloringMethodTypeIndex =
        rng->bounded(settings.allowedColoringMethodsInBlend().size());
    auto coloringMethod =
        settings.allowedColoringMethodsInBlend()[coloringMethodTypeIndex];

    Id blendId = store_.associateNewChildWith<core::System, core::Blend>(
        systemId(), [blendWeight, coloringMethod](core::Blend* blend) {
          blend->weight = blendWeight;
          blend->coloringMethod.setType(coloringMethod);
          blend->coloringMethod.params =
              generateColoringMethodParams(coloringMethod);
        });
    int numFormulas = rng->bounded(settings.minFormulasInBlend(),
                                   settings.maxFormulasInBlend() + 1);
    QVector<float> formulaWeights = generateUniformWeights(numFormulas);

    for (int j = 0; j < numFormulas; ++j) {
      // Generate formulas for blend
      int typeIndex = rng->bounded(settings.allowedFormulaTypes().size());
      auto formulaType = settings.allowedFormulaTypes()[typeIndex];
      float formulaWeight = formulaWeights[j];

      store_.associateNewChildWith<core::Blend, core::Formula>(
          blendId, [formulaType, formulaWeight](core::Formula* formula) {
            formula->setType(formulaType);
            formula->params = generateFormulaParams(formulaType);
            formula->weight.x = formulaWeight;
            formula->weight.y = formulaWeight;
          });
    }
  }

  // Randomize final blend
  int coloringMethodTypeIndex =
      rng->bounded(settings.allowedColoringMethodsInFinalBlend().size());
  auto coloringMethod =
      settings.allowedColoringMethodsInFinalBlend()[coloringMethodTypeIndex];

  int numFormulas = rng->bounded(settings.minFormulasInFinalBlend(),
                                 settings.maxFormulasInFinalBlend() + 1);
  for (int j = 0; j < numFormulas; ++j) {
    // Generate formulas for final blend
    int typeIndex = rng->bounded(settings.allowedFormulaTypes().size());
    auto formulaType = settings.allowedFormulaTypes()[typeIndex];

    store_.associateNewChildWith<core::FinalBlend, core::Formula>(
        store_.lastId<core::FinalBlend>(),
        [formulaType](core::Formula* formula) {
          formula->setType(formulaType);
          formula->params = generateFormulaParams(formulaType);
        });
  }

  store_.update<core::FinalBlend>(
      store_.lastId<core::FinalBlend>(),
      [coloringMethod](core::FinalBlend* blend) {
        blend->coloringMethod.setType(coloringMethod);
        blend->coloringMethod.params =
            generateColoringMethodParams(coloringMethod);

        // Temporary hack to make the randomized images more interesting
        // TODO: remove this one we have bounds editing
        blend->post = fromQtTransform(QTransform::fromScale(0.5, 0.5));
      });

  endResetModel();

  setModified(true);
}

void DocumentModel::moveFormulaToBlend(const QModelIndex& sourceFormula,
                                       const QModelIndex& destinationBlend) {
  if (!matchesType<core::Formula>(sourceFormula)) return;
  if (!isBlend(destinationBlend)) return;

  moveRow(parent(sourceFormula), sourceFormula.row(), destinationBlend, 0);
}

void DocumentModel::resetDocument() {
  core::Document defaultDoc;
  setItemData(documentIndex(),
              {{WidthRole, 1024},
               {HeightRole, 1024},
               {GammaRole, defaultDoc.gamma},
               {ExposureRole, defaultDoc.exposure},
               {VibrancyRole, defaultDoc.vibrancy},
               {ColorMapRole, colorMapRegistry_->defaultName()}});
  setFilePath("");
  randomizeSystem();
  setModified(true);
}

/////////////////////// QAbstractItemModel method overrides for read-only access

QHash<int, QByteArray> DocumentModel::roleNames() const {
  QHash<int, QByteArray> names = QAbstractItemModel::roleNames();
  // Common roles
  names[TypeRole] = "type";
  names[ModelIndexRole] = "modelIndex";
  // Formula-specific roles
  names[ParamsRole] = "params";
  // Blend-specific roles
  names[EnabledRole] = "enabled";
  names[PreTransformRole] = "pre";
  names[PostTransformRole] = "post";
  names[ColoringMethodTypeRole] = "coloringMethodType";
  names[ColoringMethodParamRole] = "coloringMethodParam";
  // Related to both formulas and blends
  names[SingleFormulaIndexRole] = "singleFormulaIndex";
  names[WeightRole] = "weight";
  // System-specific roles
  names[IsolatedBlendIndexRole] = "isolatedBlendIndex";
  names[TtlRole] = "ttl";
  // Document-specific roles
  names[ColorMapRole] = "colorMap";
  names[ExposureRole] = "exposure";
  names[HeightRole] = "height";
  names[GammaRole] = "gamma";
  names[WidthRole] = "width";
  names[VibrancyRole] = "vibrancy";
  return names;
}

Qt::ItemFlags DocumentModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (DocumentStore::matchesType<core::Blend>(toId(index.internalId()))) {
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
  if (DocumentStore::matchesType<core::System>(parentId)) {
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
  if (DocumentStore::matchesType<core::Blend>(*parentId)) {
    const auto& blends = store_.children<core::Blend>(*grandparentId);
    auto it = std::find(blends.begin(), blends.end(), *parentId);
    return createIndex(std::distance(blends.begin(), it), 0, fromId(*parentId));
  }
  if (DocumentStore::matchesType<core::FinalBlend>(*parentId)) {
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

QVariant documentData(const core::Document* document, int role) {
  if (!document) return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return QStringLiteral("Document");
    case DocumentModel::TypeRole:
      return DocumentEntryType::Document;
    case DocumentModel::ColorMapRole:
      return QString::fromStdString(document->colorMap);
    case DocumentModel::GammaRole:
      return document->gamma;
    case DocumentModel::ExposureRole:
      return document->exposure;
    case DocumentModel::VibrancyRole:
      return document->vibrancy;
    case DocumentModel::HeightRole:
      return document->height;
    case DocumentModel::WidthRole:
      return document->width;
    default:
      return QVariant();
  }
}
QVariant systemData(const core::System* system, int role) {
  if (!system) return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return QStringLiteral("System");
    case DocumentModel::TypeRole:
      return DocumentEntryType::System;
    case DocumentModel::TtlRole:
      return system->ttl;
    // case DocumentModel::IsolatedBlendIndexRole:
    //   Handled directly in data().
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
    case DocumentModel::EnabledRole:
      return blend->enabled;
    case DocumentModel::ColoringMethodTypeRole: {
      auto type = magic_enum::enum_name<library::ColoringMethodType>(
          blend->coloringMethod.type);
      return QString::fromUtf8(type.data(), type.size());
    }
    case DocumentModel::ColoringMethodParamRole:
      if (!blend->coloringMethod.params.empty()) {
        return blend->coloringMethod.params.front();
      }
      return QVariant();
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
    case DocumentModel::TypeRole:
      return DocumentEntryType::Blend;
    case DocumentModel::WeightRole:
      return blend->weight;
    default:
      return commonBlendData(blend, role);
  }
}
QVariant finalBlendData(const core::FinalBlend* blend, int role) {
  if (!blend) return QVariant();

  switch (role) {
    case Qt::DisplayRole:
      return QStringLiteral("Final Blend");
    case DocumentModel::TypeRole:
      return DocumentEntryType::FinalBlend;
    default:
      return commonBlendData(blend, role);
  }
}
QVariant formulaData(const core::Formula* formula, int role) {
  if (!formula) return QVariant();

  switch (role) {
    case Qt::DisplayRole: {
      auto type = magic_enum::enum_name<library::FormulaType>(formula->type);
      return QString::fromUtf8(type.data(), type.size());
    }
    case DocumentModel::TypeRole:
      return DocumentEntryType::Formula;
    case DocumentModel::ParamsRole:
      return QVariant::fromValue(formula->params);
    case DocumentModel::WeightRole:
      return formula->weight.x;
    default:
      return QVariant();
  }
}

QVector<int> setCommonBlendData(core::BlendBase* blend, const QVariant& value,
                                int role) {
  switch (role) {
    case DocumentModel::PreTransformRole:
      blend->pre = fromQtTransform(value.value<QTransform>());
      return {role};
    case DocumentModel::PostTransformRole:
      blend->post = fromQtTransform(value.value<QTransform>());
      return {role};
    case DocumentModel::EnabledRole:
      blend->enabled = value.toBool();
      return {role};
    case DocumentModel::ColoringMethodTypeRole: {
      auto type = magic_enum::enum_cast<library::ColoringMethodType>(
          value.toString().toStdString());
      if (!type) return {};

      blend->coloringMethod.setType(*type);
      // The above resets params, we need to signal that they've changed.
      return {DocumentModel::ColoringMethodTypeRole,
              DocumentModel::ColoringMethodParamRole};
    }
    case DocumentModel::ColoringMethodParamRole: {
      auto newParam = value.toFloat();
      auto& paramRef = blend->coloringMethod.params[0];
      if (paramRef != newParam) {
        paramRef = newParam;
        return {role};
      }
      return {};
    }
    default:
      return {};
  }
}
QVector<int> setBlendData(core::Blend* blend, const QVariant& value, int role) {
  if (!blend) return {};

  switch (role) {
    case Qt::EditRole:
      blend->name = value.toString().toStdString();
      return {Qt::DisplayRole, Qt::EditRole};
    case DocumentModel::WeightRole: {
      float weight = value.toFloat();
      if (!qFuzzyCompare(blend->weight, weight)) {
        blend->weight = weight;
        return {role};
      }
    }
    default:
      return setCommonBlendData(blend, value, role);
  }
}
QVector<int> setFinalBlendData(core::FinalBlend* blend, const QVariant& value,
                               int role) {
  if (!blend) return {};

  return setCommonBlendData(blend, value, role);
}
QVector<int> setFormulaData(core::Formula* formula, const QVariant& value,
                            int role) {
  if (!formula) return {};

  switch (role) {
    case DocumentModel::ParamsRole:
      formula->params = value.value<std::vector<float>>();
      return {role};
    case DocumentModel::WeightRole: {
      float val = value.toFloat();
      formula->weight.x = val;
      formula->weight.y = val;
      return {role};
    }
    default:
      return {};
  }
}

QVector<int> setSystemData(core::System* system, const QVariant& value,
                           int role) {
  if (!system) return {};

  switch (role) {
    case DocumentModel::TtlRole: {
      int ttl = value.toInt();
      if (ttl != system->ttl) {
        system->ttl = ttl;
        return {role};
      }
    }
    default:;
  }

  return {};
}

QVector<int> setDocumentData(core::Document* document, const QVariant& value,
                             int role) {
  if (!document) return {};

  switch (role) {
    case DocumentModel::ColorMapRole:
      document->colorMap = value.toString().toStdString();
      return {role};
    case DocumentModel::GammaRole:
      document->gamma = value.toFloat();
      return {role};
    case DocumentModel::ExposureRole:
      document->exposure = value.toFloat();
      return {role};
    case DocumentModel::VibrancyRole:
      document->vibrancy = value.toFloat();
      return {role};
    case DocumentModel::WidthRole:
      document->width = value.toUInt();
      return {role};
    case DocumentModel::HeightRole:
      document->height = value.toUInt();
      return {role};
    default:
      return {};
  }
}

}  // namespace

QVariant DocumentModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (role == DocumentModel::ModelIndexRole) {
    return index;
  }

  Id id = toId(index.internalId());
  if (DocumentStore::matchesType<core::Document>(id)) {
    return documentData(store_.find<core::Document>(id), role);
  }
  if (DocumentStore::matchesType<core::System>(id)) {
    const auto* system = store_.find<core::System>(id);

    if (role == DocumentModel::IsolatedBlendIndexRole) {
      if (system->isolatedBlend != nullptr) {
        const auto& blends = system->blends;
        auto it =
            std::find(blends.begin(), blends.end(), system->isolatedBlend);
        if (it != blends.end()) {
          return this->index(std::distance(blends.begin(), it), 0, index);
        }
      }
      return QModelIndex();
    }

    return systemData(system, role);
  }
  if (DocumentStore::matchesType<core::Blend>(id)) {
    if (role == DocumentModel::SingleFormulaIndexRole &&
        store_.countAllChildren(id) <= 1) {
      return formulaAt(0, index);
    }

    return blendData(store_.find<core::Blend>(id), role);
  }
  if (DocumentStore::matchesType<core::FinalBlend>(id)) {
    return finalBlendData(store_.find<core::FinalBlend>(id), role);
  }
  if (DocumentStore::matchesType<core::Formula>(id)) {
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
  QVector<int> updatedRoles;
  if (DocumentStore::matchesType<core::Blend>(id)) {
    updatedRoles =
        store_.update<core::Blend>(id, [&value, role](core::Blend* blend) {
          return setBlendData(blend, value, role);
        });
  } else if (DocumentStore::matchesType<core::FinalBlend>(id)) {
    updatedRoles = store_.update<core::FinalBlend>(
        id, [&value, role](core::FinalBlend* blend) {
          return setFinalBlendData(blend, value, role);
        });
  } else if (DocumentStore::matchesType<core::Formula>(id)) {
    updatedRoles = store_.update<core::Formula>(
        id, [&value, role](core::Formula* formula) {
          return setFormulaData(formula, value, role);
        });
  } else if (DocumentStore::matchesType<core::System>(id)) {
    if (role == DocumentModel::IsolatedBlendIndexRole) {
      auto blendIndex = value.toModelIndex();
      updatedRoles = store_.update<core::System>(
          id, [blendIndex](core::System* system) -> QVector<int> {
            const core::Blend* isolatedBlend =
                blendIndex.isValid() ? system->blends[blendIndex.row()]
                                     : nullptr;
            if (isolatedBlend != system->isolatedBlend) {
              system->isolatedBlend = isolatedBlend;
              return {DocumentModel::IsolatedBlendIndexRole};
            }
            return {};
          });
    } else {
      updatedRoles =
          store_.update<core::System>(id, [&value, role](core::System* system) {
            return setSystemData(system, value, role);
          });
    }
  } else if (DocumentStore::matchesType<core::Document>(id)) {
    updatedRoles = store_.update<core::Document>(
        id, [&value, role](core::Document* document) {
          return setDocumentData(document, value, role);
        });
  }

  // Logging setup
  static auto allRoleNames = roleNames();
  const auto& update = logUpdate();
  if (updatedRoles.isEmpty()) {
    qCWarning(update) << "failure" << toId(index)
                      << QLatin1String(allRoleNames.value(role)) << value;
  } else {
    if (update.isDebugEnabled()) {
      QVector<QLatin1String> roles;
      for (int updatedRole : updatedRoles) {
        roles.append(QLatin1String(allRoleNames.value(updatedRole)));
      }
      qDebug(update) << "success" << toId(index) << roles << value;
    }
  }

  if (!updatedRoles.isEmpty()) {
    emit dataChanged(index, index, updatedRoles);
  }
  return !updatedRoles.isEmpty();
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
  if (DocumentStore::matchesType<core::Document>(parentId)) {
    // Trying to remove the System, remove all Blends.
    auto system = systemIndex();
    return removeRows(0, rowCount(system), system);
  }

  const std::vector<Id>* children;
  if (DocumentStore::matchesType<core::System>(parentId)) {
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

  beginRemoveRows(parent, row, row + count - 1);
  for (auto id : idsToRemove) {
    store_.remove(id);
  }
  endRemoveRows();

  return true;
}

bool DocumentModel::moveRows(const QModelIndex& sourceParent, int sourceRow,
                             int count, const QModelIndex& destinationParent,
                             int destinationChild) {
  if (!sourceParent.isValid() || !destinationParent.isValid()) return false;
  if (count < 1) return false;

  // We don't support reordering.
  if (sourceParent == destinationParent) return false;

  // It's only possible to move formulas.
  if (!isBlend(sourceParent)) return false;

  Id sourceParentId = toId(sourceParent);
  Id destinationParentId = toId(destinationParent);

  // Sanity check for number of formulas.
  if (sourceRow + count > store_.countAllChildren(sourceParentId)) return false;

  // Promoting formulas to a new blend
  if (matchesType<core::System>(destinationParent)) {
    // We override destinationChild because Store doesn't support reordering
    // yet.
    destinationChild = store_.countChildren<core::Blend>(destinationParentId);

    // Add a blend and set it as new destination
    beginInsertRows(destinationParent, destinationChild, destinationChild);
    store_.associateNewChildWith<core::System, core::Blend>(
        destinationParentId);
    endInsertRows();

    return moveRowsBetweenBlends(sourceParent, sourceRow, count,
                                 index(destinationChild, 0, destinationParent));
  }

  // Moving formulas between blends
  if (DocumentStore::matchesType<core::Blend>(destinationParentId) ||
      DocumentStore::matchesType<core::FinalBlend>(destinationParentId)) {
    return moveRowsBetweenBlends(sourceParent, sourceRow, count,
                                 destinationParent);
  }

  return false;
}

bool DocumentModel::moveRowsBetweenBlends(
    const QModelIndex& sourceParent, int sourceRow, int count,
    const QModelIndex& destinationParent) {
  Id sourceParentId = toId(sourceParent);
  Id destinationParentId = toId(destinationParent);
  int destinationChild = store_.countAllChildren(destinationParentId);
  const auto& sourceChildren = store_.allChildren(sourceParentId);

  beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1,
                destinationParent, destinationChild);

  std::vector<Id> idsToMove(sourceChildren.begin() + sourceRow,
                            sourceChildren.begin() + sourceRow + count);

  for (Id id : idsToMove) {
    if (DocumentStore::matchesType<core::Blend>(sourceParentId)) {
      store_.dissociateChildFrom<core::Blend, core::Formula>(sourceParentId,
                                                             id);
    } else {
      store_.dissociateChildFrom<core::FinalBlend, core::Formula>(
          sourceParentId, id);
    }

    if (DocumentStore::matchesType<core::Blend>(destinationParentId)) {
      store_.associateChildWith<core::Blend, core::Formula>(destinationParentId,
                                                            id);
    } else {
      store_.associateChildWith<core::FinalBlend, core::Formula>(
          destinationParentId, id);
    }
  }

  endMoveRows();
  return true;
}

//////////////////////////////////////////////////////////////// Private methods

state::Id DocumentModel::documentId() const {
  return store_.lastId<core::Document>();
};

state::Id DocumentModel::systemId() const {
  return store_.lastId<core::System>();
};

bool DocumentModel::fixInvariants() {
  bool performedChanges = false;

  Id documentId;
  if (store_.count<core::Document>() < 1) {
    documentId = store_.create<core::Document>(
        [](core::Document* doc) { doc->colorMap = "Rainbow"; });
    performedChanges = true;
  } else {
    documentId = store_.lastId<core::Document>();
  }

  Id systemId;
  if (store_.countChildren<core::System>(documentId) < 1) {
    systemId =
        store_.associateNewChildWith<core::Document, core::System>(documentId);
    performedChanges = true;
  } else {
    systemId = store_.lastId<core::System>();
  }

  if (store_.countChildren<core::FinalBlend>(systemId) < 1) {
    store_.associateNewChildWith<core::System, core::FinalBlend>(systemId);
    performedChanges = true;
  }

  return performedChanges;
}
void DocumentModel::maybeUpdateBlendDisplayName(const QModelIndex& blend) {
  if (matchesType<core::Blend>(blend) &&
      data(blend, Qt::EditRole).toString().isEmpty()) {
    emit dataChanged(blend, blend, {Qt::DisplayRole});
  }
}
QModelIndex DocumentModel::getFormulaIndex(const QModelIndex& blendOrFormula) {
  if (matchesType<core::Formula>(blendOrFormula)) {
    return blendOrFormula;
  }
  if (matchesType<core::Blend>(blendOrFormula)) {
    return formulaAt(0, blendOrFormula);
  }
  return QModelIndex();
}
void DocumentModel::setModified(bool modified) {
  if (modified_ == modified) return;

  modified_ = modified;
  emit modifiedChanged();
}
void DocumentModel::setFilePath(const QString& path) {
  if (filePath_ == path) return;

  filePath_ = path;
  emit filePathChanged();
}
QString DocumentModel::name() const {
  if (filePath_.isEmpty()) {
    return QStringLiteral("Untitled");
  }
  return QFileInfo(filePath_).fileName();
}
void DocumentModel::handleDataChanges(const QModelIndex& topLeft,
                                      const QModelIndex& bottomRight,
                                      const QVector<int>& roles) {
  if (topLeft != documentIndex() && !roles.contains(Qt::DisplayRole)) {
    emit structureChanged();
  }

  setModified(true);
}
void DocumentModel::handleRowInsertion(const QModelIndex& parent, int first,
                                       int last) {
  if (parent.isValid() && parent != documentIndex()) {
    emit structureChanged();

    maybeUpdateBlendDisplayName(parent);
  }

  setModified(true);
}
void DocumentModel::handleRowRemoval(const QModelIndex& parent, int first,
                                     int last) {
  if (parent.isValid() && parent != documentIndex()) {
    emit structureChanged();

    maybeUpdateBlendDisplayName(parent);
  }

  setModified(true);
}
void DocumentModel::handleRowMove(const QModelIndex& parent, int first,
                                  int last, const QModelIndex& destination,
                                  int row) {
  if (!parent.isValid() || parent == documentIndex() ||
      !destination.isValid() || destination == documentIndex()) {
    return;
  }

  emit structureChanged();
  maybeUpdateBlendDisplayName(parent);
  maybeUpdateBlendDisplayName(destination);

  setModified(true);
}

}  // namespace chaoskit::ui
