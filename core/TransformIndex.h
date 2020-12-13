#ifndef CHAOSKIT_CORE_TRANSFORMINDEX_H
#define CHAOSKIT_CORE_TRANSFORMINDEX_H

#include <array>
#include <cstdint>
#include <ostream>
#include <stdexcept>

namespace chaoskit::core {

class TransformIndex {
 public:
  TransformIndex() = default;
  TransformIndex(std::initializer_list<uint16_t> prefix);

  [[nodiscard]] TransformIndex firstChild() const;
  [[nodiscard]] TransformIndex nextSibling() const;
  [[nodiscard]] TransformIndex parent() const;

  bool operator==(const TransformIndex& other) const;
  bool operator!=(const TransformIndex& other) const;
  bool operator<(const TransformIndex& other) const;

  [[nodiscard]] unsigned long long hash() const noexcept;

  friend std::ostream& operator<<(std::ostream& stream,
                                  const TransformIndex& index);

 private:
  constexpr static uint16_t SENTINEL = 0xffff;
  using InternalIndex = std::array<uint16_t, 16>;
  int8_t depth_ = 0;

  InternalIndex index_ = {SENTINEL, SENTINEL, SENTINEL, SENTINEL,
                          SENTINEL, SENTINEL, SENTINEL, SENTINEL,
                          SENTINEL, SENTINEL, SENTINEL, SENTINEL,
                          SENTINEL, SENTINEL, SENTINEL, SENTINEL};

  TransformIndex(std::array<uint16_t, 16> index, int depth)
      : index_(index), depth_(depth) {}
};

class InvalidTransformIndex : public std::out_of_range {
 public:
  InvalidTransformIndex(const char* message) : std::out_of_range(message) {}
};

}  // namespace chaoskit::core

template <>
struct std::hash<chaoskit::core::TransformIndex> {
  typedef chaoskit::core::TransformIndex argument_type;
  typedef unsigned long long result_type;

  result_type operator()(const argument_type& index) const noexcept;
};

#endif  // CHAOSKIT_CORE_TRANSFORMINDEX_H
