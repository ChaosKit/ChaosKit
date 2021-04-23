#ifndef CHAOSKIT_UI_MODELS_FORMULAMODEL_H
#define CHAOSKIT_UI_MODELS_FORMULAMODEL_H

#include <chaoskit.pb.h>
#include <QString>
#include <QVariantList>
#include "BaseModel.h"
#include "library/FormulaType.h"

namespace chaoskit::ui {

class FormulaModel : public BaseModel<Formula> {
  Q_OBJECT
  Q_PROPERTY(const QString& type READ type WRITE setType NOTIFY typeChanged);
  Q_PROPERTY(const QVariantList& params READ params NOTIFY paramsChanged);
  Q_PROPERTY(float weight READ weight WRITE setWeight NOTIFY weightChanged);

 public:
  FormulaModel(QObject* parent);

  void setProto(Formula* proto) override;

  [[nodiscard]] const QString& type() const { return typeCache_; }
  void setType(const QString& type);

  [[nodiscard]] const QVariantList& params() const { return paramsCache_; }
  Q_INVOKABLE void setParam(int i, float value);

  [[nodiscard]] float weight() const { return proto_->weight().x(); }
  void setWeight(float weight);

 signals:
  void typeChanged();
  void paramsChanged();
  void weightChanged();
  void invalidTypePicked();

 private:
  library::FormulaType type_;
  QString typeCache_;
  QVariantList paramsCache_;

  void updateTypeCache();
  void updateParamsCache();
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_MODELS_FORMULAMODEL_H
