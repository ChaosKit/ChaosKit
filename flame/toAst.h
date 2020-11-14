#ifndef CHAOSKIT_FLAME_TOAST_H
#define CHAOSKIT_FLAME_TOAST_H

#include "System.h"
#include "ast/Transform.h"

namespace chaoskit::flame {

ast::Transform toAst(const BlendBase& blend);
ast::Transform toAst(const System& system);

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_TOAST_H
