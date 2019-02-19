#include "Blend.h"

namespace chaoskit {
namespace ui {

void Blend::setName(const QString &name) {
  if (name == name_) {
    return;
  }

  name_ = name;
  emit nameChanged();
}

void Blend::setPre(const QTransform &pre) {
  if (qFuzzyCompare(pre, pre_)) {
    return;
  }

  pre_ = pre;
  emit preChanged();
}

void Blend::setPost(const QTransform &post) {
  if (qFuzzyCompare(post, post_)) {
    return;
  }

  post_ = post;
  emit postChanged();
}

Formula *Blend::addFormula() {
  auto *formula = new Formula();
  addFormula(formula);
  return formula;
}

void Blend::addFormula(Formula *formula) {
  formulas_.append(formula);
  emit sourceChanged();

  connect(formula, &Formula::typeChanged, this, &Blend::sourceChanged);
  connect(formula, &Formula::paramsChanged, this, &Blend::paramsChanged);
  connect(formula, &Formula::weightChanged, this, &Blend::paramsChanged);
}

void Blend::removeFormula(Formula *formula) {
  formula->disconnect(this);
  formulas_.removeOne(formula);
  emit sourceChanged();
}

void Blend::removeFormulaAt(int index) {
  formulas_.at(index)->disconnect(this);
  formulas_.remove(index);
  emit sourceChanged();
}

void Blend::setWeight(float weight) {
  if (qFuzzyCompare(weight, weight_)) {
    return;
  }

  weight_ = weight;
  emit weightChanged();
}

}  // namespace ui
}  // namespace chaoskit
