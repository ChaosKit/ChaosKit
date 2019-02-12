#ifndef CHAOSKIT_UI_BLEND_H
#define CHAOSKIT_UI_BLEND_H

#include <QObject>
#include <QPointer>
#include <QtGui/QTransform>
#include "Formula.h"

namespace chaoskit {
namespace ui {

class Blend : public QObject {
  Q_OBJECT
 public:
  const QTransform &pre() const { return pre_; }
  void setPre(const QTransform &pre);

  const QTransform &post() const { return post_; }
  void setPost(const QTransform &post);

  const QVector<QPointer<Formula>> &formulas() const { return formulas_; }
  void addFormula(Formula *formula);

  float weight() const { return weight_; }
  void setWeight(float weight);

 signals:
  void preChanged();
  void postChanged();
  void formulasChanged();
  void weightChanged();

 private:
  QTransform pre_, post_;
  QVector<QPointer<Formula>> formulas_;
  float weight_ = 1.f;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_BLEND_H
