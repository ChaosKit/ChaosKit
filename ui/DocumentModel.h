#ifndef CHAOSKIT_UI_DOCUMENTMODEL_H
#define CHAOSKIT_UI_DOCUMENTMODEL_H

#include <QAbstractItemModel>
#include "core/structures/Blend.h"
#include "core/structures/Document.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "library/FormulaType.h"
#include "state/HierarchicalStore.h"

namespace chaoskit::ui {

class DocumentModel : public QAbstractItemModel {
  Q_OBJECT

  using Store =
      state::HierarchicalStore<core::Blend, core::Document, core::FinalBlend,
                               core::Formula, core::System>;
  Store store_;

 public:
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

  void addFormula(library::FormulaType type, const QModelIndex& blendIndex);

  [[nodiscard]] QModelIndex documentIndex() const;
  [[nodiscard]] QModelIndex systemIndex() const;

 public slots:
  void addBlend();

 private:
  [[nodiscard]] state::Id documentId() const;
  [[nodiscard]] state::Id systemId() const;

  void fixInvariants();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_DOCUMENTMODEL_H
