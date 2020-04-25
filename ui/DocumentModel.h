#ifndef CHAOSKIT_UI_DOCUMENTMODEL_H
#define CHAOSKIT_UI_DOCUMENTMODEL_H

#include <QAbstractItemModel>
#include "ColorMapRegistry.h"
#include "DocumentProxy.h"
#include "DocumentStore.h"
#include "ModelEntry.h"
#include "RandomizationSettings.h"
#include "SystemProxy.h"
#include "core/structures/Blend.h"
#include "core/structures/Document.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "library/FormulaType.h"
#include "state/HierarchicalStore.h"

namespace chaoskit::ui {

class DocumentEntryType {
  Q_GADGET
 public:
  DocumentEntryType() = delete;

  enum Type {
    Blend,
    Document,
    FinalBlend,
    Formula,
    System,
  };
  Q_ENUM(Type)
};

class DocumentModel : public QAbstractItemModel {
  Q_OBJECT
  Q_PROPERTY(QString debugSource READ debugSource NOTIFY structureChanged)
  Q_PROPERTY(QModelIndex documentIndex READ documentIndex NOTIFY systemReset)
  Q_PROPERTY(QModelIndex systemIndex READ systemIndex NOTIFY systemReset)
  Q_PROPERTY(
      DocumentProxy* documentProxy READ documentProxy NOTIFY documentReset)
  Q_PROPERTY(SystemProxy* systemProxy READ systemProxy NOTIFY systemReset)
  Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
  Q_PROPERTY(QString name READ name NOTIFY filePathChanged)
  Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)

  DocumentStore store_;

 public:
  enum Roles {
    // Common roles
    TypeRole = Qt::UserRole + 1,
    ModelIndexRole,
    // Formula-specific roles
    ParamsRole,
    // Blend-specific roles
    EnabledRole,
    PreTransformRole,
    PostTransformRole,
    ColoringMethodTypeRole,
    ColoringMethodParamsRole,
    // Related to both formulas and blends
    SingleFormulaIndexRole,
    WeightRole,
    // System-specific roles
    TtlRole,
    // Document-specific roles
    ColorMapRole,
    ExposureRole,
    GammaRole,
    HeightRole,
    VibrancyRole,
    WidthRole,
  };

  explicit DocumentModel(QObject* parent = nullptr);

  // QAbstractItemModel method overrides for read access
  [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index,
                              int role) const override;
  [[nodiscard]] bool hasChildren(const QModelIndex& parent) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role) const override;
  [[nodiscard]] QModelIndex index(int row, int column,
                                  const QModelIndex& parent) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] Q_INVOKABLE int rowCount(
      const QModelIndex& parent) const override;

  // QAbstractItemModel method overrides for write access
  bool setData(const QModelIndex& index, const QVariant& value,
               int role) override;
  bool removeRows(int row, int count, const QModelIndex& parent) override;
  bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                const QModelIndex& destinationParent,
                int destinationChild) override;

  // Custom API
  template <typename T>
  [[nodiscard]] bool matchesType(const QModelIndex& index) const {
    if (!index.isValid()) {
      return false;
    }
    return DocumentStore::matchesType<T>(state::Id(index.internalId()));
  }
  [[nodiscard]] bool isBlend(const QModelIndex& index) const;
  [[nodiscard]] Q_INVOKABLE bool isFinalBlend(const QModelIndex& index) const;
  [[nodiscard]] QModelIndex blendAt(int i) const;
  [[nodiscard]] QModelIndex formulaAt(int i,
                                      const QModelIndex& blendIndex) const;

  QModelIndex addFormula(library::FormulaType type,
                         const QModelIndex& blendIndex);
  Q_INVOKABLE QModelIndex addFormula(const QString& type,
                                     const QModelIndex& blendIndex);

  QModelIndex addBlend(library::FormulaType type);
  Q_INVOKABLE QModelIndex addBlend(const QString& type);

  Q_INVOKABLE bool removeRowAtIndex(const QModelIndex& index);

  Q_INVOKABLE void absorbBlend(const QModelIndex& source,
                               const QModelIndex& destination);
  Q_INVOKABLE void moveFormulaToBlend(const QModelIndex& sourceFormula,
                                      const QModelIndex& destinationBlend);

  void adoptDocument(core::Document* document);
  Q_INVOKABLE bool loadFromFile(const QString& path);
  Q_INVOKABLE bool saveToFile(const QString& path);

  [[nodiscard]] QModelIndex documentIndex() const;
  [[nodiscard]] QModelIndex systemIndex() const;
  [[nodiscard]] QModelIndex finalBlendIndex() const;

  [[nodiscard]] const core::Document* document() const;
  [[nodiscard]] const core::System* system() const;

  [[nodiscard]] Q_INVOKABLE ModelEntry* entryAtIndex(const QModelIndex& index);
  [[nodiscard]] DocumentProxy* documentProxy();
  [[nodiscard]] SystemProxy* systemProxy();

  [[nodiscard]] QString debugSource() const;

  void setColorMapRegistry(ColorMapRegistry* colorMapRegistry) {
    colorMapRegistry_ = colorMapRegistry;
  }

  [[nodiscard]] bool isModified() const { return modified_; }
  [[nodiscard]] const QString& filePath() const { return filePath_; }
  [[nodiscard]] QString name() const;

 public slots:
  void resetDocument();
  void randomizeParams(const QModelIndex& index);
  void randomizeSystem();
  void randomizeSystem(const RandomizationSettings& settings);

 signals:
  void structureChanged();
  void systemReset();
  void documentReset();
  void ioFailed(const QString& error);

  void modifiedChanged();
  void filePathChanged();

 private:
  DocumentProxy* documentProxy_;
  SystemProxy* systemProxy_;
  bool modified_ = true;
  QString filePath_ = "";
  ColorMapRegistry* colorMapRegistry_;

  [[nodiscard]] state::Id documentId() const;
  [[nodiscard]] state::Id systemId() const;

  void setModified(bool modified);
  void setFilePath(const QString& path);

  bool fixInvariants();
  void maybeUpdateBlendDisplayName(const QModelIndex& blend);
  QModelIndex getFormulaIndex(const QModelIndex& blendOrFormula);
  bool moveRowsBetweenBlends(const QModelIndex& sourceParent, int sourceRow,
                             int count, const QModelIndex& destinationParent);

 private slots:
  void handleDataChanges(const QModelIndex& topLeft,
                         const QModelIndex& bottomRight,
                         const QVector<int>& roles);
  void handleRowInsertion(const QModelIndex& parent, int first, int last);
  void handleRowRemoval(const QModelIndex& parent, int first, int last);
  void handleRowMove(const QModelIndex& parent, int first, int last,
                     const QModelIndex& destination, int row);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_DOCUMENTMODEL_H
