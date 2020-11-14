#ifndef CHAOSKIT_CORE_UTIL_H
#define CHAOSKIT_CORE_UTIL_H

#include <ast/Blend.h>
#include <ast/Formula.h>
#include <ast/System.h>
#include <stdx/optional.h>
#include "System.h"
#include "SystemIndex.h"

namespace chaoskit::flame {

stdx::optional<size_t> formulaIndex(const ast::Formula &formula,
                                    const ast::Blend &blend);
stdx::optional<SystemIndex> formulaIndex(const ast::Formula &formula,
                                         const ast::System &system);

std::string debugString(const System &system);

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_CORE_UTIL_H
