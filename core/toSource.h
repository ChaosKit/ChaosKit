#ifndef CHAOSKIT_CORE_TOSOURCE_H
#define CHAOSKIT_CORE_TOSOURCE_H

#include <ast/System.h>
#include <ast/Transform.h>
#include <ast/WeightedFormula.h>
#include "models/Model.h"
#include "structures/Blend.h"
#include "structures/Formula.h"
#include "structures/System.h"
#include "structures/Transform.h"

namespace chaoskit::core {

ast::Blend toSource(const Blend &blend);
ast::WeightedFormula toSource(const Formula &formula);
ast::Transform toSource(const Transform &transform);
ast::System toSource(const System &system);

template <class T>
auto toSource(const Model<T> &model) -> decltype(toSource(T())) {
  return toSource(*model.data().lock());
}

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TOSOURCE_H
