#include "debug.h"
#include "ast/transform_debug.h"

namespace chaoskit::core {

std::ostream& operator<<(std::ostream& stream, const Point& point) {
  return stream << "(" << point.x() << ", " << point.y() << ")";
}

std::ostream& operator<<(std::ostream& stream, const Particle& particle) {
  return stream << "[" << particle.point << ", " << particle.color << "]";
}

std::ostream& operator<<(std::ostream& stream, const TransformParams& params) {
  stream << "Params:";
  for (const auto& [index, list] : params) {
    stream << std::endl << "  " << index << " = ";

    bool isFirst = true;
    for (float param : list) {
      if (!isFirst) stream << ", ";
      stream << param;
      isFirst = false;
    }
  }
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const TransformSystem& system) {
  return stream << "Transform:" << std::endl
                << system.transform << std::endl
                << system.params;
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const stdx::optional<T>& opt) {
  if (opt) {
    return stream << *opt;
  }
  return stream << "nullopt";
}

std::ostream& operator<<(std::ostream& stream,
                         const CameraSystem& cameraSystem) {
  stream << "CameraSystem(" << cameraSystem.system << ", "
         << cameraSystem.camera << ", " << cameraSystem.lifetime << ")";
  return stream;
}

}  // namespace chaoskit::core
