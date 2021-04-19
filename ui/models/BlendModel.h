#ifndef CHAOSKIT_UI_MODELS_BLENDMODEL_H
#define CHAOSKIT_UI_MODELS_BLENDMODEL_H

#include <QObject>
#include "BaseModel.h"
#include "ModelFactory.h"
#include "TransformModel.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class BlendModel : public QObject, public BaseModel<Blend> {
  Q_OBJECT
  Q_PROPERTY(const QString& name READ name WRITE setName NOTIFY nameChanged);
  Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged);
  Q_PROPERTY(float weight READ weight WRITE setWeight NOTIFY weightChanged);
  Q_PROPERTY(TransformModel* pre READ pre NOTIFY preChanged);
  Q_PROPERTY(TransformModel* post READ post NOTIFY postChanged);

 public:
  explicit BlendModel(ModelFactory* modelFactory, QObject* parent = nullptr);

  void setProto(Blend* proto) override;

  [[nodiscard]] bool enabled() const { return proto_->enabled(); }
  void setEnabled(bool enabled);

  [[nodiscard]] float weight() const { return proto_->weight(); }
  void setWeight(float weight);

  [[nodiscard]] const QString& name() const { return nameCache_; }
  void setName(const QString& name);

  [[nodiscard]] TransformModel* pre() const { return pre_; }
  [[nodiscard]] TransformModel* post() const { return post_; }

 public slots:
  void addPre();
  void removePre();
  void addPost();
  void removePost();

 signals:
  void enabledChanged();
  void weightChanged();
  void nameChanged();
  void preChanged();
  void postChanged();

 private:
  ModelFactory* modelFactory_;
  QString nameCache_;
  TransformModel* pre_ = nullptr;
  TransformModel* post_ = nullptr;

  void updatePre();
  void updatePost();
  void updateNameCache();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_BLENDMODEL_H
