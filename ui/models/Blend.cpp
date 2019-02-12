#include "Blend.h"

namespace chaoskit {
namespace ui {

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

void Blend::addFormula(Formula *formula) {
  formulas_.append(formula);
  emit formulasChanged();
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
