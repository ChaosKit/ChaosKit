#ifndef CHAOSKIT_UI_TOSOURCE_H
#define CHAOSKIT_UI_TOSOURCE_H

#include <ast/Blend.h>
#include <ast/System.h>
#include <ast/Transform.h>
#include <ast/WeightedFormula.h>
#include <QTransform>
#include "Blend.h"
#include "Formula.h"
#include "System.h"

namespace chaoskit {
namespace ui {

ast::Blend toSource(const Blend *blend);
ast::WeightedFormula toSource(const Formula *formula);
ast::Transform toSource(const QTransform &transform);
ast::System toSource(const System *system);

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_TOSOURCE_H
