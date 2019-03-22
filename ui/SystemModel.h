#ifndef CHAOSKIT_UI_SYSTEMMODEL_H
#define CHAOSKIT_UI_SYSTEMMODEL_H

#include <QAbstractItemModel>
#include "FlatteningModel.h"
#include "models/Blend.h"
#include "models/Formula.h"
#include "models/System.h"

namespace chaoskit {
namespace ui {

class SystemModel : public QAbstractItemModel {
  Q_OBJECT
  Q_PROPERTY(chaoskit::ui::System *system READ system NOTIFY systemChanged)
 public:
  enum SystemRoles {
    WeightRole = Qt::UserRole + 1,
    BlendIndexRole,
    IsFinalBlendRole,
  };
  explicit SystemModel(QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex &parent) const override;
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;
  bool setHeaderData(int section, Qt::Orientation orientation,
                     const QVariant &value, int role) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  bool insertRows(int row, int count, const QModelIndex &parent) override;
  bool removeRows(int row, int count, const QModelIndex &parent) override;

  Q_INVOKABLE FlatteningModel *childModel(int index);
  Q_INVOKABLE QModelIndex modelIndexForSelection(int index);
  Q_INVOKABLE void addBlend();
  Q_INVOKABLE void addFormula(int blendIndex, const QString &type);
  Q_INVOKABLE void removeRowAtIndex(const QModelIndex &index);
  Q_INVOKABLE bool isFinalBlend(const QModelIndex &index);

  System *system() const { return system_; }

 signals:
  void systemChanged();

 private:
  System *system_;
  library::FormulaType formulaTypeForAdding = library::FormulaType::Invalid;

  Blend *getBlendForId(uint64_t id) const;

  QVariant blendData(const Blend *blend, int role) const;
  bool setBlendData(Blend *blend, int role, const QVariant &value) const;
  QVariant formulaData(const Formula *formula, int role) const;
  bool setFormulaData(Formula *blend, int role, const QVariant &value) const;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_SYSTEMMODEL_H
