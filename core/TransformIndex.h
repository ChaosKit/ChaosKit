#ifndef CHAOSKIT_CORE_TRANSFORMINDEX_H
#define CHAOSKIT_CORE_TRANSFORMINDEX_H

#include <array>

namespace chaoskit::core {

class TransformIndex {
 public:
  TransformIndex() = default;
  explicit TransformIndex(std::array<uint16_t, 16> index) : index_(index) {}
  TransformIndex(std::initializer_list<uint16_t> prefix);

  [[nodiscard]] TransformIndex firstChild() const;
  [[nodiscard]] TransformIndex nextSibling() const;
  [[nodiscard]] TransformIndex parent() const;

  bool operator==(const TransformIndex& other) const;
  bool operator!=(const TransformIndex& other) const;
  bool operator<(const TransformIndex& other) const;

 private:
  constexpr static uint16_t SENTINEL = 0xffff;
  using InternalIndex = std::array<uint16_t, 16>;

  InternalIndex index_ = {SENTINEL, SENTINEL, SENTINEL, SENTINEL,
                          SENTINEL, SENTINEL, SENTINEL, SENTINEL,
                          SENTINEL, SENTINEL, SENTINEL, SENTINEL,
                          SENTINEL, SENTINEL, SENTINEL, SENTINEL};

  [[nodiscard]] int getDepth() const;
};

}  // namespace chaoskit::core

#endif  // CHAOSKIT_CORE_TRANSFORMINDEX_H
