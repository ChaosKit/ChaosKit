#ifndef CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H
#define CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H

#include <memory>
#include "System.h"

namespace chaoskit::core {

struct [[deprecated]] Document {
  System* system;

  float gamma = 2.2f;
  float exposure = 0.0f;
  float vibrancy = 0.0f;
  std::string colorMap;
  uint32_t width;
  uint32_t height;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_STRUCTURES_DOCUMENT_H
