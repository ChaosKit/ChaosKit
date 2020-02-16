#ifndef CHAOSKIT_CORE_SYSTEMINDEX_H
#define CHAOSKIT_CORE_SYSTEMINDEX_H

#include <cstddef>
#include <functional>
#include <limits>

namespace chaoskit::core {

struct SystemIndex {
  static const size_t FINAL_BLEND = std::numeric_limits<size_t>::max();
  static const size_t COLORING_METHOD = std::numeric_limits<size_t>::max();

  size_t blend = 0;
  size_t formula = 0;

  bool operator==(const SystemIndex &other) const {
    return blend == other.blend && formula == other.formula;
  }
};

}  // namespace chaoskit::core

namespace std {
template <>
struct hash<chaoskit::core::SystemIndex> {
  typedef chaoskit::core::SystemIndex argument_type;
  typedef std::size_t result_type;

  result_type operator()(const argument_type &index) const noexcept {
    return (index.formula & 0xffff) | (index.blend << 16);
  }
};
}  // namespace std

#endif  // CHAOSKIT_CORE_SYSTEMINDEX_H
