#ifndef CHAOSKIT_UI_MODELS_BASEMODEL_H
#define CHAOSKIT_UI_MODELS_BASEMODEL_H

namespace chaoskit::ui {

template <class Proto>
class BaseModel {
 public:
  [[nodiscard]] const Proto* proto() const { return proto_; }
  virtual void setProto(Proto* proto) { proto_ = proto; }

 protected:
  Proto* proto_ = nullptr;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_BASEMODEL_H
