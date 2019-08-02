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

    addChild(parentId, childId);
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
    addChild(parentId, childId);
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
            it = parents_.erase(it);
          }
        }
      });
    }

    if constexpr (isParent<T>()) {
      ChildrenOf<T>::List::forEach([this, id](auto type) {
        using ChildType = std::remove_pointer_t<decltype(type)>;

        if constexpr (Store<Ts...>::template containsType<ChildType>()) {
          auto it = children_.find(id);
          if (it == children_.end()) {
            return;
          }

          auto& children = it->second;
          for (auto childIt = children.begin(); childIt != children.end();) {
            if (Store<Ts...>::template matchesType<ChildType>(*childIt)) {
              remove<ChildType>(*childIt);
              childIt = children.erase(childIt);
            } else {
              ++childIt;
            }
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
      auto it = children_.find(parentId);
      if (it == children_.end()) {
        return 0;
      }
      return std::count_if(
          it->second.begin(), it->second.end(), [](const Id& child) {
            return Store<Ts...>::template matchesType<C>(child);
          });
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

  // TODO: make this not return vectors ;_;
  template <typename C>
  [[nodiscard]] std::vector<Id> children(Id parentId) const {
    std::vector<Id> result;
    if constexpr (isChild<C>()) {
      auto it = children_.find(parentId);
      if (it != children_.end()) {
        std::copy_if(it->second.begin(), it->second.end(),
                     std::back_inserter(result), [](const Id& id) {
                       return Store<Ts...>::template matchesType<C>(id);
                     });
      }
    }
    return result;
  }

  [[nodiscard]] const Id* parent(Id childId) const {
    auto it = parents_.find(childId);
    if (it != parents_.end()) {
      return &it->second;
    }
    return nullptr;
  }

 private:
  void addChild(Id key, Id value) {
    auto it = children_.find(key);
    if (it == children_.end()) {
      children_.emplace(key, std::vector<Id>{value});
    } else {
      it->second.push_back(value);
    }
  }
};

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_HIERARCHICALSTORE_H
