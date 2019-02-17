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
  Q_PROPERTY(
      chaoskit::ui::models::System *system READ system NOTIFY systemChanged)
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

  models::System *system() const { return system_; }

 signals:
  void systemChanged();

 private:
  models::System *system_;

  const Blend *getBlendForId(uint64_t id) const;

  QVariant blendData(const Blend *blend, bool isFinal, int column,
                     int role) const;
  QVariant formulaData(const Formula *formula, int column, int role) const;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_SYSTEMMODEL_H
