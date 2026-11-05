#ifndef CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
#define CHAOSKIT_UI_MODELS_SYSTEMMODEL_H

#include <QQmlObjectListModel.h>
#include <chaoskit.pb.h>
#include <QObject>
#include "BaseModel.h"
#include "BlendModel.h"
#include "ModelFactory.h"
#include "core/SystemParticle.h"

namespace chaoskit::ui {

class SystemModel : public BaseModel<System> {
  Q_OBJECT
  Q_PROPERTY(QQmlObjectListModelBase* blends READ blends CONSTANT);
  Q_PROPERTY(BlendModel* cameraBlend READ cameraBlend CONSTANT);
  Q_PROPERTY(int minLifetime READ minLifetime WRITE setMinLifetime NOTIFY
                 minLifetimeChanged);
  Q_PROPERTY(int maxLifetime READ maxLifetime WRITE setMaxLifetime NOTIFY
                 maxLifetimeChanged);
  Q_PROPERTY(
      bool isInfiniteLifetime READ isInfiniteLifetime WRITE setInfiniteLifetime
          NOTIFY isInfiniteLifetimeChanged STORED false);
  Q_PROPERTY(const QString& astSource READ astSource NOTIFY visuallyChanged);
  Q_PROPERTY(const QString& modelSource READ modelSource NOTIFY protoChanged);

 public:
  SystemModel(ModelFactory* modelFactory, QObject* parent = nullptr);

  void setProto(System* proto) override;

  [[nodiscard]] int minLifetime() const { return proto_->min_lifetime(); }
  void setMinLifetime(int min);

  [[nodiscard]] int maxLifetime() const { return proto_->max_lifetime(); }
  void setMaxLifetime(int max);

  [[nodiscard]] bool isInfiniteLifetime() const {
    return proto_->max_lifetime() == core::SystemParticle::INFINITE_LIFETIME;
  }
  void setInfiniteLifetime(bool infinite);

  [[nodiscard]] QQmlObjectListModel<BlendModel>* blends() const {
    return blends_;
  }
  [[nodiscard]] BlendModel* cameraBlend() const { return cameraBlend_; }

  [[nodiscard]] QString astSource() const;
  [[nodiscard]] QString modelSource() const;

 public slots:
  void addBlend();
  void deleteBlendAt(int index);

 signals:
  void minLifetimeChanged();
  void maxLifetimeChanged();
  void isInfiniteLifetimeChanged();
  void visuallyChanged();

 private:
  ModelFactory* modelFactory_;
  QQmlObjectListModel<BlendModel>* blends_;
  BlendModel* cameraBlend_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
