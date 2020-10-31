#include "TransformIndex.h"
#include <algorithm>
#include "errors.h"

namespace chaoskit::core {

TransformIndex::TransformIndex(std::initializer_list<uint16_t> prefix) {
  std::copy(prefix.begin(), prefix.end(), index_.begin());
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

int TransformIndex::getDepth() const {
  for (int i = 0; i < index_.size(); i++) {
    if (index_[i] == SENTINEL) return i;
  }
  return index_.size();
}

TransformIndex TransformIndex::firstChild() const {
  int depth = getDepth();
  if (depth >= index_.size()) {
    throw InvalidTransformIndex("Transform depth limit reached");
  }

  InternalIndex newIndex(index_);
  newIndex[depth] = 0;
  return TransformIndex(newIndex);
}
TransformIndex TransformIndex::nextSibling() const {
  int level = getDepth() - 1;
  if (level < 0) {
    throw InvalidTransformIndex("Root transform cannot have siblings");
  }
  if (index_[level] == SENTINEL - 1) {
    throw InvalidTransformIndex(
        "Cannot have more than 65535 transforms per level");
  }

  InternalIndex newIndex(index_);
  newIndex[level]++;
  return TransformIndex(newIndex);
}
TransformIndex TransformIndex::parent() const {
  int level = getDepth() - 1;
  if (level < 0) {
    throw InvalidTransformIndex("Root transform does not have a parent");
  }

  InternalIndex newIndex(index_);
  newIndex[level] = SENTINEL;
  return TransformIndex(newIndex);
}

}  // namespace chaoskit::core
