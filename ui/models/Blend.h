#ifndef CHAOSKIT_UI_BLEND_H
#define CHAOSKIT_UI_BLEND_H

#include <library/FormulaType.h>
#include <QObject>
#include <QPointer>
#include <QtGui/QTransform>
#include "Formula.h"

namespace chaoskit::ui {

class Blend : public QObject {
  Q_OBJECT
 public:
  Blend(QObject *parent = nullptr) : QObject(parent) {}

  const QString &name() const { return name_; }
  void setName(const QString &name);

  const QTransform &pre() const { return pre_; }
  void setPre(const QTransform &pre);

  const QTransform &post() const { return post_; }
  void setPost(const QTransform &post);

  const QVector<QPointer<Formula>> &formulas() const { return formulas_; }
  Formula *addFormula(library::FormulaType type);
  void addFormula(Formula *formula);
  Formula *formulaAt(int index) const { return formulas_.at(index); }
  int formulaCount() const { return formulas_.size(); }
  void removeFormula(Formula *formula);
  void removeFormulaAt(int index);

  float weight() const { return weight_; }
  void setWeight(float weight);

 signals:
  void nameChanged();
  void preChanged();
  void postChanged();
  void sourceChanged();
  void paramsChanged();
  void weightChanged();

 private:
  QString name_;
  QTransform pre_, post_;
  QVector<QPointer<Formula>> formulas_;
  float weight_ = 1.f;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_BLEND_H
