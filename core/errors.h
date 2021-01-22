#ifndef CHAOSKIT_CORE_ERRORS_H
#define CHAOSKIT_CORE_ERRORS_H

#include <stdexcept>
#include "flame/SystemIndex.h"

namespace chaoskit::core {

class OutputNotAvailable : public std::invalid_argument {
 public:
  OutputNotAvailable();
};

class MissingParameterError : public std::out_of_range {
 public:
  explicit MissingParameterError(size_t parameterIndex);
};

class UndefinedVariableError : public std::out_of_range {
 public:
  UndefinedVariableError(const std::string& name);
};

class InvalidColorMap : public std::out_of_range {
 public:
  InvalidColorMap(const std::string& name);
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_ERRORS_H
