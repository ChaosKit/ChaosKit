#ifndef CHAOSKIT_UI_FORMULA_H
#define CHAOSKIT_UI_FORMULA_H

#include <core/Formula.h>
#include <library/FormulaType.h>
#include <QObject>
#include <QVector>

namespace chaoskit {
namespace ui {

class Formula : public QObject {
  Q_OBJECT
 public:
  const QString &name() const { return name_; }
  void setName(const QString &name);

  const library::FormulaType &type() const { return type_; }
  void setType(const QString &typeName);
  void setType(const library::FormulaType &type);

  const QVector<float> &params() const { return params_; }
  void setParams(const QVector<float> &params);

  const float weightX() const { return weight_x_; }
  void setWeightX(float weight);

  const float weightY() const { return weight_y_; }
  void setWeightY(float weight);

  core::Formula toCoreFormula() const;

 signals:
  void nameChanged();
  void typeChanged();
  void paramsChanged();
  void weightChanged();

 private:
  QString name_;
  library::FormulaType type_;
  QVector<float> params_;
  float weight_x_ = 1.f;
  float weight_y_ = 1.f;

  void expandParams(int size);
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_FORMULA_H
