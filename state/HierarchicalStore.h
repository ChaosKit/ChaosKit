#ifndef CHAOSKIT_STATE_HIERARCHICALSTORE_H
#define CHAOSKIT_STATE_HIERARCHICALSTORE_H

#include <unordered_map>
#include <utility>
#include <vector>
#include "Store.h"
#include "hierarchy.h"

namespace chaoskit::state {

/** Store that manages a predefined hierarchy of types. */
template <typename... Ts>
class HierarchicalStore : public Store<Ts...> {
  std::unordered_map<Id, std::vector<Id>> children_;
  std::unordered_map<Id, std::vector<std::vector<Id>>> childrenByType_;
  std::unordered_multimap<Id, Id> parents_;

 public:
  template <typename P, typename C>
  void associateChildWith(Id parentId, Id childId) {
    static_assert(Store<Ts...>::template containsType<P>());
    static_assert(Store<Ts...>::template containsType<C>());
    static_assert(isValidRelation<P, C>());

    if (!this->template matchesType<P>(parentId)) {
      throw IdTypeMismatchError("parentId in HierarchicalStore::addChild()");
    }
    if (!this->template matchesType<C>(childId)) {
      throw IdTypeMismatchError("childId in HierarchicalStore::addChild()");
    }

    auto* child = const_cast<C*>(this->template find<C>(childId));
    if (!child) {
      throw MissingIdError("childId in HierarchicalStore::addChild()");
    }

    addChild<C>(parentId, childId);
    parents_.emplace(childId, parentId);
    this->template update<P>(
        parentId, [child](P* parent) { associateChild(*parent, child); });
  }

  template <typename P, typename C>
  Id associateNewChildWith(Id parentId) {
    static_assert(Store<Ts...>::template containsType<P>());
    static_assert(Store<Ts...>::template containsType<C>());
    static_assert(isValidRelation<P, C>());

    if (!this->template matchesType<P>(parentId)) {
      throw IdTypeMismatchError("parentId in HierarchicalStore::addNewChild()");
    }

    const Id childId = this->template create<C>();
    auto* child = const_cast<C*>(this->template find<C>(childId));
    addChild<C>(parentId, childId);
    parents_.emplace(childId, parentId);
    this->template update<P>(
        parentId, [child](P* parent) { associateChild(*parent, child); });
    return childId;
  }

  template <typename P, typename C, typename Fn>
  Id associateNewChildWith(Id parentId, Fn&& updater) {
    static_assert(Store<Ts...>::template containsType<P>());
    static_assert(Store<Ts...>::template containsType<C>());
    static_assert(isValidRelation<P, C>());

    if (!this->template matchesType<P>(parentId)) {
      throw IdTypeMismatchError("parentId in HierarchicalStore::addNewChild()");
    }

    const Id childId = this->template create<C>(std::forward<Fn>(updater));
    auto* child = const_cast<C*>(this->template find<C>(childId));
    addChild<C>(parentId, childId);
    parents_.emplace(childId, parentId);
    this->template update<P>(
        parentId, [child](P* parent) { associateChild(*parent, child); });
    return childId;
  }

  template <typename T>
  void remove(Id id) {
    static_assert(Store<Ts...>::template containsType<T>());
    if (!this->template matchesType<T>(id)) {
      throw IdTypeMismatchError("in HierarchicalStore::remove()");
    }

    if constexpr (isChild<T>()) {
      ParentsOf<T>::List::forEach([this, id](auto type) {
        using ParentType = std::remove_pointer_t<decltype(type)>;

        if constexpr (Store<Ts...>::template containsType<ParentType>()) {
          for (auto it = parents_.begin(); it != parents_.end();) {
            if (it->first != id ||
                !this->template matchesType<ParentType>(it->second)) {
              ++it;
              continue;
            }

            T* child = const_cast<T*>(this->template find<T>(id));
            this->template update<ParentType>(it->second,
                                              [child](ParentType* parent) {
                                                dissociateChild(*parent, child);
                                              });
            it = cut(it->second, id, typeIndex<T>());
          }
        }
      });
    }

    if constexpr (isParent<T>()) {
      ChildrenOf<T>::List::forEach([this, id](auto type) {
        using ChildType = std::remove_pointer_t<decltype(type)>;

        if constexpr (Store<Ts...>::template containsType<ChildType>()) {
          auto it = childrenByType_.find(id);
          if (it == childrenByType_.end()) {
            return;
          }

          // The copy is necessary because the recursive call calls cut(), which
          // alters the original vector. Iterators are going to be invalidated
          // and the temporary copy prevents breakage caused by that.
          auto childrenCopy = it->second.at(typeIndex<ChildType>());
          for (const auto& childId : childrenCopy) {
            remove<ChildType>(childId);
          }
        }
      });
    }

    Store<Ts...>::template remove<T>(id);
  }

  // TODO: implement clear<T>()

  void clearAll() {
    parents_.clear();
    children_.clear();
    Store<Ts...>::clearAll();
  }

  // TODO: implement unit tests for those
  [[nodiscard]] bool hasAnyChildren(Id parentId) const {
    return children_.find(parentId) != children_.end();
  }

  template <typename C>
  [[nodiscard]] size_t countChildren(Id parentId) const {
    if constexpr (isChild<C>()) {
      auto it = childrenByType_.find(parentId);
      if (it == childrenByType_.end()) {
        return 0;
      }
      return it->second.at(typeIndex<C>()).size();
    } else {
      return 0;
    }
  }

  [[nodiscard]] size_t countAllChildren(Id parentId) const {
    auto it = children_.find(parentId);
    if (it == children_.end()) {
      return 0;
    } else {
      return it->second.size();
    }
  }

  template <typename C>
  [[nodiscard]] const std::vector<Id>* children(Id parentId) const {
    if constexpr (isChild<C>()) {
      auto it = childrenByType_.find(parentId);
      if (it == childrenByType_.end()) {
        return nullptr;
      }

      return &(it->second.at(typeIndex<C>()));
    } else {
      return nullptr;
    }
  }

  [[nodiscard]] const std::vector<Id>* allChildren(Id parentId) const {
    auto it = children_.find(parentId);
    if (it == children_.end()) {
      return nullptr;
    }
    return &it->second;
  }

  [[nodiscard]] const Id* parent(Id childId) const {
    auto it = parents_.find(childId);
    if (it != parents_.end()) {
      return &it->second;
    }
    return nullptr;
  }

 private:
  template <typename T>
  static constexpr int typeIndex() {
    return Store<Ts...>::Entity::template which<T>();
  }

  template <typename T>
  void addChild(Id key, Id value) {
    // All children
    {
      auto it = children_.find(key);
      if (it == children_.end()) {
        children_.emplace(key, std::vector<Id>{value});
      } else {
        it->second.push_back(value);
      }
    }

    // By type
    {
      auto it = childrenByType_.find(key);
      if (it == childrenByType_.end()) {
        it = childrenByType_
                 .emplace(key, std::vector<std::vector<Id>>(sizeof...(Ts)))
                 .first;
      }
      auto& typeArray = it->second;
      typeArray.at(typeIndex<T>()).push_back(value);
    }
  }

  /** Removes a connection between a parent and a child. */
  auto cut(Id parent, Id child, int childType) ->
      typename decltype(parents_)::iterator {
    // Remove child from children_[parent]
    auto childrenIt = children_.find(parent);
    if (childrenIt != children_.end()) {
      auto& children = childrenIt->second;
      children.erase(std::remove(children.begin(), children.end(), child),
                     children.end());
    }

    // Remove child from childrenByType_[parent][childType]
    auto childrenByTypeIt = childrenByType_.find(parent);
    if (childrenByTypeIt != childrenByType_.end()) {
      auto& children = childrenByTypeIt->second.at(childType);
      children.erase(std::remove(children.begin(), children.end(), child),
                     children.end());
    }

    // Remove parent in parents_[child]
    auto allParents = parents_.equal_range(child);
    for (auto it = allParents.first; it != allParents.second; it++) {
      if (it->second == parent) {
        return parents_.erase(it);
      }
    }
    return parents_.end();
  }
};

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_HIERARCHICALSTORE_H
