#ifndef CHAOSKIT_UI_SYSTEMMODEL_H
#define CHAOSKIT_UI_SYSTEMMODEL_H

#include <QAbstractItemModel>
#include "System.h"

namespace chaoskit {
namespace ui {

class SystemModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  explicit SystemModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent) const override;
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent) const override;
  QModelIndex parent(const QModelIndex &child) const override;

 private:
  core::System system_;

  QVariant blendData(const core::Blend &blend, int column, int role) const;
  QVariant finalBlendData(const core::Blend &blend, int column, int role) const;
  QVariant formulaData(const core::Formula &formula, int column,
                       int role) const;
  QVariant preTransformData(const core::Transform &transform, int column,
                            int role) const;
  QVariant postTransformData(const core::Transform &transform, int column,
                             int role) const;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_SYSTEMMODEL_H
