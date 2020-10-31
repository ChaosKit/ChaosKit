#include "errors.h"
#include <string>

namespace chaoskit::core {

OutputNotAvailable::OutputNotAvailable()
    : std::invalid_argument("Output is not yet available") {}

MissingParameterError::MissingParameterError(size_t parameterIndex)
    : std::out_of_range("Tried to access missing parameter " +
                        std::to_string(parameterIndex)) {}

MissingParameterError::MissingParameterError(const SystemIndex& elementIndex,
                                             size_t parameterIndex)
    : std::out_of_range("Tried to access missing parameter " +
                        std::to_string(parameterIndex) + " at formula " +
                        std::to_string(elementIndex.formula) + " in blend " +
                        std::to_string(elementIndex.blend)) {}

UndefinedVariableError::UndefinedVariableError(const std::string& name)
    : std::out_of_range("Tried to access undefined variable " + name) {}

InvalidColorMap::InvalidColorMap(const std::string& name)
    : std::out_of_range("Color map '" + name + "' does not exist") {}

}  // namespace chaoskit::core
