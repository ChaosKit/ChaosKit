#ifndef CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
#define CHAOSKIT_UI_MODELS_SYSTEMMODEL_H

#include <QQmlObjectListModel.h>
#include <chaoskit.pb.h>
#include <QObject>
#include "BaseModel.h"
#include "BlendModel.h"
#include "ModelFactory.h"

namespace chaoskit::ui {

class SystemModel : public QObject, public BaseModel<System> {
  Q_OBJECT
  Q_PROPERTY(QQmlObjectListModelBase* blends READ blends CONSTANT);
  Q_PROPERTY(const QString& astSource READ astSource NOTIFY somethingChanged);
  Q_PROPERTY(
      const QString& modelSource READ modelSource NOTIFY somethingChanged);

 public:
  SystemModel(ModelFactory* modelFactory, QObject* parent = nullptr);

  void setProto(System* proto) override;

  [[nodiscard]] QQmlObjectListModel<BlendModel>* blends() const {
    return blends_;
  }

  [[nodiscard]] QString astSource() const;
  [[nodiscard]] QString modelSource() const;

 signals:
  void somethingChanged();

 private:
  ModelFactory* modelFactory_;
  QQmlObjectListModel<BlendModel>* blends_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
