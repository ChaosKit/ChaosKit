#ifndef CHAOSKIT_STATE_ID_H
#define CHAOSKIT_STATE_ID_H

#include <cstdint>
#include <functional>

namespace chaoskit::state {

struct Id {
  uint32_t type;
  uint32_t id;

  Id() : type(0), id(0) {}
  explicit Id(uint64_t value) : type(value >> 32u), id(value & 0xffffffff) {}
  Id(uint32_t type, uint32_t id) : type(type), id(id) {}

  bool operator==(const Id& other) const noexcept {
    return other.type == type && other.id == id;
  }

  bool operator!=(const Id& other) const noexcept {
    return other.type != type || other.id != id;
  }

  explicit operator uint64_t() const {
    return (static_cast<uint64_t>(type) << 32u) | id;
  }
};

}  // namespace chaoskit::state

namespace std {
template <>
struct hash<chaoskit::state::Id> {
  using argument_type = chaoskit::state::Id;
  using result_type = std::uint64_t;

  result_type operator()(const argument_type& id) const noexcept {
    return static_cast<result_type>(id);
  }
};
}  // namespace std

#endif  // CHAOSKIT_STATE_ID_H
