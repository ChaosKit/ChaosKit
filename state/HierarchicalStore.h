#ifndef CHAOSKIT_STATE_HIERARCHICALSTORE_H
#define CHAOSKIT_STATE_HIERARCHICALSTORE_H

#include <unordered_map>
#include "Store.h"
#include "hierarchy.h"

namespace chaoskit::state {

/** Store that manages a predefined hierarchy of types. */
template <typename... Ts>
class HierarchicalStore : public Store<Ts...> {
  std::unordered_multimap<Id, Id> children_;
  std::unordered_map<Id, Id> parents_;

 public:
  template <typename P>
  void associateChildWith(Id parentId, Id childId) {
    static_assert(isParent<P>());
    using ChildType = typename ChildOf<P>::Type;

    if (!this->template matchesType<P>(parentId)) {
      throw IdTypeMismatchError("parentId in HierarchicalStore::addChild()");
    }
    if (!this->template matchesType<ChildType>(childId)) {
      throw IdTypeMismatchError("childId in HierarchicalStore::addChild()");
    }

    auto* child =
        const_cast<ChildType*>(this->template find<ChildType>(childId));
    if (!child) {
      throw MissingIdError("childId in HierarchicalStore::addChild()");
    }

    children_.emplace(parentId, childId);
    parents_.emplace(childId, parentId);
    this->template update<P>(
        parentId, [child](P* parent) { associateChild(*parent, child); });
  }

  template <typename P>
  Id associateNewChildWith(Id parentId) {
    static_assert(isParent<P>());
    using ChildType = typename ChildOf<P>::Type;

    if (!this->template matchesType<P>(parentId)) {
      throw IdTypeMismatchError("parentId in HierarchicalStore::addNewChild()");
    }

    const Id childId = this->template create<ChildType>();
    auto* child =
        const_cast<ChildType*>(this->template find<ChildType>(childId));
    children_.emplace(parentId, childId);
    parents_.emplace(childId, parentId);
    this->template update<P>(
        parentId, [child](P* parent) { associateChild(*parent, child); });
    return childId;
  }

  template <typename T>
  void remove(Id id) {
    if (!this->template matchesType<T>(id)) {
      throw IdTypeMismatchError("in HierarchicalStore::remove()");
    }

    if constexpr (isChild<T>()) {
      using ParentType = typename ParentOf<T>::Type;
      if constexpr (Store<Ts...>::template containsType<ParentType>()) {
        auto parentIt = parents_.find(id);
        if (parentIt != parents_.end()) {
          T* child = const_cast<T*>(this->template find<T>(id));
          this->template update<ParentType>(
              parentIt->second,
              [child](ParentType* parent) { dissociateChild(*parent, child); });
          parents_.erase(parentIt);
        }
      }
    }

    if constexpr (isParent<T>()) {
      using ChildType = typename ChildOf<T>::Type;
      if constexpr (Store<Ts...>::template containsType<ChildType>()) {
        // We can't use equal_range here because it might get invalidated when
        // we remove entities recursively.
        for (;;) {
          auto it = children_.find(id);
          if (it == children_.end()) {
            break;
          }
          remove<ChildType>(it->second);
          children_.erase(it);
        }
      }
    }

    Store<Ts...>::template remove<T>(id);
  }
};

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_HIERARCHICALSTORE_H
