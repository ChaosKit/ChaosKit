#ifndef CHAOSKIT_UI_FORMULA_H
#define CHAOSKIT_UI_FORMULA_H

#include <core/Formula.h>
#include <library/FormulaType.h>
#include <QObject>
#include <QVector>

namespace chaoskit::ui {

class Formula : public QObject {
  Q_OBJECT
  Q_PROPERTY(float weightX READ weightX WRITE setWeightX NOTIFY weightChanged)
  Q_PROPERTY(float weightY READ weightY WRITE setWeightY NOTIFY weightChanged)
 public:
  explicit Formula(QObject *parent = nullptr) : QObject(parent) {}

  const library::FormulaType &type() const { return type_; }
  void setType(const QString &typeName);
  void setType(const library::FormulaType &type);

  const QVector<float> &params() const { return params_; }
  void setParams(const QVector<float> &params);

  const float weightX() const { return weight_x_; }
  void setWeightX(float weight);

  const float weightY() const { return weight_y_; }
  void setWeightY(float weight);

 signals:
  void typeChanged();
  void paramsChanged();
  void weightChanged();

 private:
  library::FormulaType type_ = library::FormulaType::Invalid;
  QVector<float> params_;
  float weight_x_ = 1.f;
  float weight_y_ = 1.f;

  void expandParams(int size);
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_FORMULA_H
