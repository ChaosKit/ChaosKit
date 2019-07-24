#ifndef CHAOSKIT_STATE_STORE_H
#define CHAOSKIT_STATE_STORE_H

#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <mapbox/variant.hpp>
#include "Id.h"

namespace chaoskit::state {

namespace detail {

template <typename T, typename... Ts>
struct Contains : std::disjunction<std::is_same<T, Ts>...> {};

}  // namespace detail

class IdTypeMismatchError : public std::logic_error {
 public:
  explicit IdTypeMismatchError(const char* what) : std::logic_error(what) {}
};

class MissingIdError : public std::logic_error {
 public:
  explicit MissingIdError(const char* what) : std::logic_error(what) {}
};

template <typename... Ts>
class Store {
 public:
  using Entity = mapbox::util::variant<Ts...>;

  struct Changes {
    std::unordered_set<Id> created;
    std::unordered_map<Id, Entity> updated;
    std::unordered_map<Id, Entity> removed;

    void merge(Changes& other) {
      // TODO: update to use unordered_{map,set}::merge() once Apple supports it

      for (auto it = other.created.begin(); it != other.created.end(); ++it) {
        created.insert(std::move(other.created.extract(it)));
      }

      for (auto it = other.updated.begin(); it != other.updated.end(); ++it) {
        updated.insert(std::move(other.updated.extract(it)));
      }

      for (auto it = other.removed.begin(); it != other.removed.end(); ++it) {
        removed.insert(std::move(other.removed.extract(it)));
      }
    }

    void reset() {
      created.clear();
      updated.clear();
      removed.clear();
    }
  };

  Store() : entities_(), counters_(sizeof...(Ts), 0), currentTransaction_() {}

  template <typename T>
  Id create() {
    Id id = nextId<T>();
    entities_.emplace(id, T());

    if (trackingChanges()) {
      currentChanges().created.insert(id);
    }

    return id;
  }

  template <typename T, typename Fn>
  Id create(Fn updater) {
    Id id = nextId<T>();
    auto [it, ok] = entities_.emplace(id, T());

    T* entity = &mapbox::util::get_unchecked<T>(it->second);
    updater(entity);

    if (trackingChanges()) {
      currentChanges().created.insert(id);
    }

    return id;
  }

  template <typename T>
  Id lastId() const {
    uint32_t type = Entity::template which<T>();
    return {type + 1, counters_[type]};
  }

  [[nodiscard]] constexpr bool isValid(const Id& id) const {
    return id.type > 0 && id.id > 0 && id.type <= sizeof...(Ts);
  }

  template <typename T>
  const T* find(Id id) const {
    if (!matchesType<T>(id)) {
      throw IdTypeMismatchError("in Store::find()");
    }

    auto it = entities_.find(id);
    if (it == entities_.end()) {
      return nullptr;
    }

    return &mapbox::util::get_unchecked<T>(it->second);
  }

  [[nodiscard]] bool has(Id id) const {
    return entities_.find(id) != entities_.end();
  }

  template <typename T>
  [[nodiscard]] size_t count() const {
    size_t result = 0;
    for (auto it = entities_.begin(); it != entities_.end(); it++) {
      if (matchesType<T>(it->first)) {
        result++;
      }
    }
    return result;
  }

  [[nodiscard]] size_t size() const { return entities_.size(); }

  template <typename T, typename Fn>
  auto update(Id id, Fn updater)
      -> decltype(updater(static_cast<T*>(nullptr))) {
    if (!matchesType<T>(id)) {
      throw IdTypeMismatchError("in Store::update()");
    }

    auto it = entities_.find(id);
    if (it == entities_.end()) {
      throw MissingIdError("in Store::update()");
    }

    if (trackingChanges()) {
      auto& updated = currentChanges().updated;
      auto updatedIt = updated.find(it->first);
      if (updatedIt == updated.end()) {
        updated.insert(*it);
      }
    }

    T* entity = &mapbox::util::get_unchecked<T>(it->second);
    return updater(entity);
  }

  template <typename T>
  void remove(Id id) {
    if (!matchesType<T>(id)) {
      throw IdTypeMismatchError("in Store::remove()");
    }

    auto it = entities_.find(id);
    if (it == entities_.end()) {
      throw MissingIdError("in Store::remove()");
    }

    if (trackingChanges()) {
      auto node = entities_.extract(it);
      currentChanges().removed.insert(std::move(node));
    } else {
      entities_.erase(it);
    }
  }

  template <typename T>
  void clear() {
    for (auto it = entities_.begin(); it != entities_.end();) {
      if (matchesType<T>(it->first)) {
        if (trackingChanges()) {
          currentChanges().removed.insert(std::move(*it));
        }
        it = entities_.erase(it);
      } else {
        it++;
      }
    }
  }

  void clearAll() {
    if (trackingChanges()) {
      currentChanges().removed.insert(entities_.begin(), entities_.end());
    }
    entities_.clear();
  }

  template <typename Fn>
  bool transaction(Fn runner) {
    currentTransaction_ = std::make_unique<Changes>();
    bool success = true;
    try {
      runner();
    } catch (std::logic_error& e) {
      {
        auto& removed = currentTransaction_->removed;
        entities_.insert(removed.begin(), removed.end());
      }

      {
        auto& updated = currentTransaction_->updated;
        for (auto it = updated.begin(); it != updated.end(); it++) {
          entities_[it->first] = std::move(it->second);
        }
      }

      for (auto id : currentTransaction_->created) {
        entities_.erase(id);
      }

      success = false;
    }
    if (trackingChanges_) {
      changes_.merge(*currentTransaction_);
    }
    currentTransaction_.reset();
    return success;
  }

  template <typename T>
  constexpr static bool containsType() {
    return detail::Contains<T, Ts...>::value;
  }

  const Changes& changes() const { return changes_; }
  void resetChanges() { changes_.reset(); }

  void setTrackingChanges(bool trackingChanges) {
    trackingChanges_ = trackingChanges;
  }
  bool trackingChanges() const {
    return trackingChanges_ || currentTransaction_;
  }

 protected:
  template <typename T>
  [[nodiscard]] bool matchesType(Id id) const {
    if constexpr (containsType<T>()) {
      return Entity::template which<T>() + 1 == id.type;
    } else {
      return false;
    }
  }

 private:
  std::unordered_map<Id, Entity> entities_;
  std::vector<uint32_t> counters_;
  std::unique_ptr<Changes> currentTransaction_;
  Changes changes_;
  bool trackingChanges_ = false;

  Changes& currentChanges() {
    if (currentTransaction_) {
      return *currentTransaction_;
    }
    return changes_;
  }

  template <typename T>
  Id nextId() {
    return nextId(Entity::template which<T>());
  }

  Id nextId(uint32_t type) { return {type + 1, nextCounter(type)}; }

  uint32_t nextCounter(uint32_t type) { return ++counters_[type]; }
};

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_STORE_H
