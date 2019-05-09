#include "Point.h"

namespace chaoskit::core {

std::ostream &operator<<(std::ostream &stream, const Point &point) {
  return stream << "(" << point.x() << ", " << point.y() << ")";
}

}  // namespace chaoskit::core
