#ifndef CHAOSKIT_CORE_ERRORS_H
#define CHAOSKIT_CORE_ERRORS_H

#include <stdexcept>
#include "SystemIndex.h"

namespace chaoskit::core {

class MissingParameterError : public std::out_of_range {
 public:
  MissingParameterError(const SystemIndex& elementIndex, size_t parameterIndex);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_ERRORS_H
