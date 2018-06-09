#include "Point.h"

namespace chaoskit {
namespace core {

std::ostream &operator<<(std::ostream &stream, const Point &point) {
  return stream << "(" << point.x() << ", " << point.y() << ")";
}

}  // namespace core
}  // namespace chaoskit
