#ifndef CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
#define CHAOSKIT_UI_MODELS_SYSTEMMODEL_H

#include <QQmlObjectListModel.h>
#include <chaoskit.pb.h>
#include <QObject>
#include "BaseModel.h"
#include "BlendModel.h"
#include "ModelFactory.h"

namespace chaoskit::ui {

class SystemModel : public BaseModel<System> {
  Q_OBJECT
  Q_PROPERTY(QQmlObjectListModelBase* blends READ blends CONSTANT);
  Q_PROPERTY(BlendModel* cameraBlend READ cameraBlend CONSTANT);
  Q_PROPERTY(int minLifetime READ minLifetime WRITE setMinLifetime NOTIFY
                 minLifetimeChanged);
  Q_PROPERTY(int maxLifetime READ maxLifetime WRITE setMaxLifetime NOTIFY
                 maxLifetimeChanged);
  Q_PROPERTY(bool isImmortal READ isImmortal WRITE setImmortal NOTIFY
                 isImmortalChanged STORED false);
  Q_PROPERTY(const QString& astSource READ astSource NOTIFY protoChanged);
  Q_PROPERTY(const QString& modelSource READ modelSource NOTIFY protoChanged);

 public:
  SystemModel(ModelFactory* modelFactory, QObject* parent = nullptr);

  void setProto(System* proto) override;

  [[nodiscard]] int minLifetime() const { return proto_->skip(); }
  void setMinLifetime(int min);

  [[nodiscard]] int maxLifetime() const { return proto_->ttl(); }
  void setMaxLifetime(int max);

  [[nodiscard]] bool isImmortal() const { return proto_->ttl() == -1; }
  void setImmortal(bool immortal);

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
  void isImmortalChanged();

 private:
  ModelFactory* modelFactory_;
  QQmlObjectListModel<BlendModel>* blends_;
  BlendModel* cameraBlend_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_SYSTEMMODEL_H
