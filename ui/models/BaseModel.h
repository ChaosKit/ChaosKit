#ifndef CHAOSKIT_UI_MODELS_BASEMODEL_H
#define CHAOSKIT_UI_MODELS_BASEMODEL_H

#include <QObject>

namespace chaoskit::ui {

class AbstractBaseModel : public QObject {
  Q_OBJECT
 public:
  AbstractBaseModel(QObject* parent = nullptr) : QObject(parent) {}

 signals:
  void protoChanged();
};

template <class Proto>
class BaseModel : public AbstractBaseModel {
 public:
  explicit BaseModel(QObject* parent = nullptr) : AbstractBaseModel(parent) {}

  [[nodiscard]] const Proto* proto() const { return proto_; }
  virtual void setProto(Proto* proto) { proto_ = proto; }

 protected:
  Proto* proto_ = nullptr;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_BASEMODEL_H
