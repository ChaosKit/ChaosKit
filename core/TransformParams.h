#ifndef CHAOSKIT_CORE_TRANSFORMPARAMS_H
#define CHAOSKIT_CORE_TRANSFORMPARAMS_H

#include <unordered_map>
#include <vector>
#include "TransformIndex.h"

namespace chaoskit::core {

using TransformParams = std::unordered_map<TransformIndex, std::vector<float>>;

}

#endif  // CHAOSKIT_CORE_TRANSFORMPARAMS_H
