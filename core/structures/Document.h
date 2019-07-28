#ifndef CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H
#define CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H

#include <memory>
#include "System.h"
#include "state/hierarchy.h"

namespace chaoskit::core {

struct Document {
  System* system;
};

}  // namespace chaoskit::core

CHAOSKIT_DEFINE_SIMPLE_RELATION(::chaoskit::core::Document,
                                ::chaoskit::core::System,
                                &chaoskit::core::Document::system);

#endif  // CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H
