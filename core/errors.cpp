#include "errors.h"
#include <string>

namespace chaoskit::core {

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
