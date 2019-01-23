#ifndef CHAOSKIT_UI_SYSTEMMODEL_H
#define CHAOSKIT_UI_SYSTEMMODEL_H

#include <QAbstractItemModel>
#include "FlatteningModel.h"
#include "System.h"

namespace chaoskit {
namespace ui {

class SystemModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  enum SystemRoles {
    WeightRole = Qt::UserRole + 1,
  };
  explicit SystemModel(QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex &parent) const override;
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent) const override;
  QModelIndex parent(const QModelIndex &child) const override;

  Q_INVOKABLE FlatteningModel *childModel(int index);

 private:
  core::System system_;

  QVariant blendData(const core::Blend &blend, bool isFinal, int column,
                     int role) const;
  QVariant formulaData(const core::Formula &formula, int column,
                       int role) const;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_SYSTEMMODEL_H
