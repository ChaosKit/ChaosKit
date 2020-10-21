#ifndef CHAOSKIT_CORE_TOSOURCE_H
#define CHAOSKIT_CORE_TOSOURCE_H

#include <ast/StaticAffineTransform.h>
#include <ast/System.h>
#include <ast/WeightedFormula.h>
#include "structures/Blend.h"
#include "structures/Formula.h"
#include "structures/System.h"
#include "structures/Transform.h"

namespace chaoskit::core {

ast::Blend toSource(const BlendBase &blend);
ast::WeightedFormula toSource(const Formula &formula);
ast::Expression toSource(const ColoringMethod &coloringMethod);
ast::StaticAffineTransform toSource(const Transform &transform);
ast::System toSource(const System &system);

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TOSOURCE_H
