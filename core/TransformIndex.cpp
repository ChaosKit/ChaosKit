#include "TransformIndex.h"
#include <xxhash.h>
#include <algorithm>
#include "errors.h"

namespace chaoskit::core {

TransformIndex::TransformIndex(std::initializer_list<uint16_t> prefix) {
  if (prefix.size() > index_.size()) {
    throw InvalidTransformIndex(
        "Cannot create a TransformIndex with more than 16 levels");
  }

  std::copy(prefix.begin(), prefix.end(), index_.begin());
  depth_ = prefix.size();
}

bool TransformIndex::operator==(const TransformIndex& other) const {
  for (int i = 0; i < index_.size(); i++) {
    const auto& a = index_[i];
    const auto& b = other.index_[i];

    if (a != b) {
      return false;
    }
    if (a == SENTINEL) {
      break;
    }
  }
  return true;
}

bool TransformIndex::operator!=(const TransformIndex& other) const {
  return !(*this == other);
}

bool TransformIndex::operator<(const TransformIndex& other) const {
  for (int i = 0; i < index_.size(); i++) {
    const auto& a = index_[i];
    const auto& b = other.index_[i];

    // !(a < b) if b is empty, they are at best equal
    if (b == SENTINEL) {
      return false;
    }
    // a is empty, b is not => a < b
    if (a == SENTINEL) {
      return true;
    }
    // a != b, so the answer lies in front of us.
    if (a != b) {
      return a < b;
    }
    // a == b => check further indices.
  }
  // We checked all indices and they were all equal.
  return false;
}

TransformIndex TransformIndex::firstChild() const {
  if (depth_ >= index_.size()) {
    throw InvalidTransformIndex("Transform depth limit reached");
  }

  InternalIndex newIndex(index_);
  newIndex[depth_] = 0;
  return TransformIndex(newIndex, depth_ + 1);
}
TransformIndex TransformIndex::nextSibling() const {
  int level = depth_ - 1;
  if (level < 0) {
    throw InvalidTransformIndex("Root transform cannot have siblings");
  }
  if (index_[level] == SENTINEL - 1) {
    throw InvalidTransformIndex(
        "Cannot have more than 65535 transforms per level");
  }

  InternalIndex newIndex(index_);
  newIndex[level]++;
  return TransformIndex(newIndex, depth_);
}
TransformIndex TransformIndex::parent() const {
  int level = depth_ - 1;
  if (level < 0) {
    throw InvalidTransformIndex("Root transform does not have a parent");
  }

  InternalIndex newIndex(index_);
  newIndex[level] = SENTINEL;
  return TransformIndex(newIndex, level);
}

unsigned long long TransformIndex::hash() const noexcept {
  return XXH3_64bits(index_.data(), index_.size() * sizeof(uint16_t));
}

}  // namespace chaoskit::core

unsigned long long std::hash<chaoskit::core::TransformIndex>::operator()(
    const chaoskit::core::TransformIndex& index) const noexcept {
  return index.hash();
}
