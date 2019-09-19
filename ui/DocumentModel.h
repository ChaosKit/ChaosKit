#ifndef CHAOSKIT_UI_DOCUMENTMODEL_H
#define CHAOSKIT_UI_DOCUMENTMODEL_H

#include <QAbstractItemModel>
#include "ModelEntry.h"
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
  Q_PROPERTY(
      QModelIndex documentIndex READ documentIndex NOTIFY invariantsFixed)
  Q_PROPERTY(QModelIndex systemIndex READ systemIndex NOTIFY invariantsFixed)

  using Store =
      state::HierarchicalStore<core::Blend, core::Document, core::FinalBlend,
                               core::Formula, core::System>;
  Store store_;

 public:
  enum Roles {
    ParamsRole = Qt::UserRole + 1,
    PreTransformRole,
    PostTransformRole,
    WeightRole,
    TypeRole,
    EnabledRole,
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
  [[nodiscard]] int rowCount(const QModelIndex& parent) const override;

  // QAbstractItemModel method overrides for write access
  bool setData(const QModelIndex& index, const QVariant& value,
               int role) override;
  bool removeRows(int row, int count, const QModelIndex& parent) override;

  // Custom API
  template <typename T>
  [[nodiscard]] bool matchesType(const QModelIndex& index) const {
    if (!index.isValid()) {
      return false;
    }
    return Store::matchesType<T>(state::Id(index.internalId()));
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

  [[nodiscard]] QModelIndex documentIndex() const;
  [[nodiscard]] QModelIndex systemIndex() const;
  [[nodiscard]] QModelIndex finalBlendIndex() const;

  [[nodiscard]] const core::Document* document() const;
  [[nodiscard]] const core::System* system() const;

  [[nodiscard]] Q_INVOKABLE ModelEntry* entryAtIndex(const QModelIndex& index);

  [[nodiscard]] QString debugSource() const;

 public slots:
  void randomizeParams(const QModelIndex& index);

 signals:
  void structureChanged();
  void invariantsFixed();

 private:
  [[nodiscard]] state::Id documentId() const;
  [[nodiscard]] state::Id systemId() const;

  void fixInvariants();

 private slots:
  void handleDataChanges(const QModelIndex& topLeft,
                         const QModelIndex& bottomRight,
                         const QVector<int>& roles);
  void handleRowInsertion(const QModelIndex& parent, int first, int last);
  void handleRowRemoval(const QModelIndex& parent, int first, int last);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_DOCUMENTMODEL_H
