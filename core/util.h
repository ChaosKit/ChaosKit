#ifndef CHAOSKIT_CORE_UTIL_H
#define CHAOSKIT_CORE_UTIL_H

#include <ast/Blend.h>
#include <ast/Formula.h>
#include <ast/System.h>
#include <stdx/optional.h>
#include "SystemIndex.h"

namespace chaoskit::core {

stdx::optional<size_t> formulaIndex(const ast::Formula &formula,
                                    const ast::Blend &blend);
stdx::optional<SystemIndex> formulaIndex(const ast::Formula &formula,
                                         const ast::System &system);

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_UTIL_H
