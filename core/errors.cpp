#include "errors.h"
#include <string>

namespace chaoskit::core {

OutputNotAvailable::OutputNotAvailable()
    : std::invalid_argument("Output is not yet available") {}

MissingParameterError::MissingParameterError(size_t parameterIndex)
    : std::out_of_range("Tried to access missing parameter " +
                        std::to_string(parameterIndex)) {}

UndefinedVariableError::UndefinedVariableError(const std::string& name)
    : std::out_of_range("Tried to access undefined variable " + name) {}

InvalidColorMap::InvalidColorMap(const std::string& name)
    : std::out_of_range("Color map '" + name + "' does not exist") {}

}  // namespace chaoskit::core
