#ifndef CHAOSKIT_CORE_ERRORS_H
#define CHAOSKIT_CORE_ERRORS_H

#include <stdexcept>
#include "SystemIndex.h"

namespace chaoskit::core {

class MissingParameterError : public std::out_of_range {
 public:
  MissingParameterError(const SystemIndex& elementIndex, size_t parameterIndex);
};

class InvalidColorMap : public std::out_of_range {
 public:
  InvalidColorMap(const std::string& name);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_ERRORS_H
