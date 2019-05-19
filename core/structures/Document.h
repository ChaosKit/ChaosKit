#ifndef CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H
#define CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H

#include <memory>
#include "System.h"

namespace chaoskit::core {

struct Document {
  std::shared_ptr<System> system;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H
