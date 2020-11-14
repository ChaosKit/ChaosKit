#ifndef CHAOSKIT_FLAME_DOCUMENT_H
#define CHAOSKIT_FLAME_DOCUMENT_H

#include <memory>
#include "System.h"

namespace chaoskit::flame {

struct Document {
  System* system;

  float gamma = 2.2f;
  float exposure = 0.0f;
  float vibrancy = 0.0f;
  std::string colorMap;
  uint32_t width;
  uint32_t height;
};

}  // namespace chaoskit::flame

#endif  // CHAOSKIT_FLAME_DOCUMENT_H
