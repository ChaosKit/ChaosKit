#ifndef CHAOSKIT_CORE_TOSOURCE_H
#define CHAOSKIT_CORE_TOSOURCE_H

#include <ast/Transform.h>
#include <ast/WeightedFormula.h>
#include <ast/System.h>
#include "Formula.h"
#include "Transform.h"
#include "Blend.h"
#include "System.h"

namespace chaoskit {
namespace core {

ast::Blend toSource(const Blend &blend);
ast::WeightedFormula toSource(const Formula &formula);
ast::Transform toSource(const Transform &transform);
ast::System toSource(const System &system);

}  // namespace core
}  // namespace chaoskit

#endif  // CHAOSKIT_CORE_TOSOURCE_H
