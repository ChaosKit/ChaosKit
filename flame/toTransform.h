#ifndef CHAOSKIT_FLAME_TOTRANSFORM_H
#define CHAOSKIT_FLAME_TOTRANSFORM_H

#include "System.h"
#include "ast/Transform.h"
#include "stdx/optional.h"

namespace chaoskit::flame {

ast::Transform toTransform(const BlendBase& blend);
ast::Transform toTransform(const System& system);
stdx::optional<ast::Transform> toCameraTransform(const System& system);

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_TOTRANSFORM_H
