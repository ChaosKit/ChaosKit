#ifndef CHAOSKIT_STATE_ID_H
#define CHAOSKIT_STATE_ID_H

#include <cstdint>
#include <functional>

namespace chaoskit::state {

struct Id {
  uint32_t type = 0;
  uint32_t id = 0;

  bool operator==(const Id& other) const noexcept {
    return other.type == type && other.id == id;
  }

  bool operator!=(const Id& other) const noexcept {
    return other.type != type || other.id != id;
  }
};

}  // namespace chaoskit::state

namespace std {
template <>
struct hash<chaoskit::state::Id> {
  using argument_type = chaoskit::state::Id;
  using result_type = std::size_t;

  result_type operator()(const argument_type& id) const noexcept {
    return (static_cast<result_type>(id.type) << 32u) | id.id;
  }
};
}  // namespace std

#endif  // CHAOSKIT_STATE_ID_H
