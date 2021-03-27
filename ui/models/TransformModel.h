#ifndef CHAOSKIT_UI_MODELS_TRANSFORMMODEL_H
#define CHAOSKIT_UI_MODELS_TRANSFORMMODEL_H

#include <QObject>
#include "BaseModel.h"
#include "chaoskit.pb.h"

namespace chaoskit::ui {

class TransformModel : public QObject, public BaseModel<Transform> {
  Q_OBJECT
  Q_PROPERTY(float scaleX READ scaleX WRITE setScaleX NOTIFY scaleXChanged);
  Q_PROPERTY(float scaleY READ scaleY WRITE setScaleY NOTIFY scaleYChanged);
  Q_PROPERTY(
      float rotation READ rotation WRITE setRotation NOTIFY rotationChanged);
  Q_PROPERTY(float translationX READ translationX WRITE setTranslationX NOTIFY
                 translationXChanged);
  Q_PROPERTY(float translationY READ translationY WRITE setTranslationY NOTIFY
                 translationYChanged);

 public:
  TransformModel(QObject* parent = nullptr);

  void setProto(Transform* proto) override;

  [[nodiscard]] float scaleX() const { return scaleX_; }
  void setScaleX(float sx);
  [[nodiscard]] float scaleY() const { return scaleY_; }
  void setScaleY(float sy);
  [[nodiscard]] float rotation() const { return rotation_; }
  void setRotation(float angle);
  [[nodiscard]] float translationX() const { return translationX_; }
  void setTranslationX(float dx);
  [[nodiscard]] float translationY() const { return translationY_; }
  void setTranslationY(float dy);

 signals:
  void transformChanged();
  void rotationChanged();
  void scaleXChanged();
  void scaleYChanged();
  void translationXChanged();
  void translationYChanged();

 private:
  float rotation_;
  float scaleX_;
  float scaleY_;
  float translationX_;
  float translationY_;

  void updateMatrix();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_TRANSFORMMODEL_H
